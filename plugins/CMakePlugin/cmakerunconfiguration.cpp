#include "cmakerunconfiguration.h"

#include <QJsonArray>
#include <QJsonObject>

#include "buildjobs/cmakebuildjob.h"
#include "buildjobs/cmakeconfigurejob.h"

#include "fileapiclient/replyindex.h"
#include "project/presets/processrunjob.h"

struct CmakeRunConfigurationPrivate {
        QWeakPointer<Project> project;
        CmakeBuildEngine* buildEngine;
        QString name;

        QDir buildDirectory;
        QStringList cmakeArgs;
        QProcessEnvironment cmakeEnv;
};

CmakeRunConfiguration::CmakeRunConfiguration(ProjectPtr project, CmakeBuildEngine* buildEngine, QString name, QJsonObject descriptor, QObject* parent) :
    RunConfiguration{parent} {
    d = new CmakeRunConfigurationPrivate();
    d->project = project;
    d->name = name;
    d->buildEngine = buildEngine;
    d->cmakeEnv = QProcessEnvironment::systemEnvironment();

    d->buildDirectory = project->projectDir(Project::BuildRootDirectory).absoluteFilePath(name.toLower().replace(" ", "_"));

    for (auto arg : descriptor.value("cmake-args").toArray()) {
        d->cmakeArgs.append(arg.toString());
    }

    connect(project.data(), &Project::runConfigurationsUpdated, this, [=]() -> QCoro::Task<> {
        if (project->activeRunConfiguration() == this) {
            // Restart clangd with this run configuration
            auto lsp = co_await project->languageServerForServerName("clangd");
            QJsonObject extraInitialisationOptions = {
                {"compilationDatabasePath", d->buildDirectory.absolutePath() + "/"}
            };
            co_await lsp->startLanguageServer(extraInitialisationOptions);
        }
    });

#ifdef Q_OS_WIN
    // Attempt to find MSVC, Ninja and the WinSDK and add it to the PATH
    QDir root("C:/");
    QStringList extraPaths;
    QStringList extraLibs;

    for (auto dir : this->glob(root, {"Program Files", "Program Files (x86)"})) {
        // Find Visual Studio stuff
        for (auto year : this->glob(dir.absoluteFilePath("Microsoft Visual Studio"), {"2022", "2019", "2017"})) {
            for (auto edition : this->glob(year, {"Community", "Professional", "Enterprise"})) {
                // Find MSVC
                auto msvcRoot = QDir(edition.absoluteFilePath("VC/Tools/MSVC"));
                if (msvcRoot.exists()) {
                    for (auto version : msvcRoot.entryList()) {
                        auto binDir = QDir(msvcRoot.absoluteFilePath(version));
                        binDir = binDir.absoluteFilePath("bin");
                        for (auto hostArch : this->glob(binDir, {"Hostx64", "Hostx86"})) {
                            for (auto arch : this->glob(hostArch, {"x64", "x86", "arm64", "arm"})) {
                                extraPaths.append(arch.absolutePath());
                            }
                        }

                        auto libDir = QDir(msvcRoot.absoluteFilePath(version));
                        libDir = libDir.absoluteFilePath("lib");
                        libDir = libDir.absoluteFilePath("onecore");
                        for (auto arch : this->glob(libDir, {"x64", "x86", "arm64", "arm"})) {
                            extraLibs.append(arch.absolutePath());
                        }
                    }
                }

                // Find ninja
                auto ninjaRoot = QDir(edition.absoluteFilePath("Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja"));
                if (ninjaRoot.exists()) {
                    extraPaths.append(ninjaRoot.absolutePath());
                }
            }
        }

        // Find Windows SDK
        auto kitsBinDir = QDir(dir.absoluteFilePath("Windows Kits/10/bin"));
        for (auto kit : kitsBinDir.entryList()) {
            if (!kit.startsWith("10")) continue;
            auto kitDir = QDir(kitsBinDir.absoluteFilePath(kit));
            for (auto arch : this->glob(kitDir, {"x64", "x86", "arm64", "arm"})) {
                extraPaths.append(arch.absolutePath());
            }
        }
        auto kitsLibDir = QDir(dir.absoluteFilePath("Windows Kits/10/Lib"));
        for (auto kit : kitsLibDir.entryList()) {
            if (!kit.startsWith("10")) continue;
            auto kitDir = QDir(kitsLibDir.absoluteFilePath(kit));
            for (auto libDir : this->glob(kitDir, {"ucrt", "um"})) {
                for (auto arch : this->glob(libDir, {"x64", "x86", "arm64", "arm"})) {
                    extraLibs.append(arch.absolutePath());
                }
            }
        }
    }

    extraPaths.append(d->cmakeEnv.value("Path").split(";"));
    d->cmakeEnv.insert("Path", extraPaths.join(";"));
    extraLibs.append(d->cmakeEnv.value("Lib").split(";"));
    d->cmakeEnv.insert("Lib", extraLibs.join(";"));
#endif
}

CmakeRunConfiguration::~CmakeRunConfiguration() {
    delete d;
}

QCoro::Generator<QDir> CmakeRunConfiguration::glob(QDir dir, QStringList possibilities) {
    for (auto path : possibilities) {
        QDir test = dir.absoluteFilePath(path);
        if (test.exists()) co_yield test.absolutePath();
    }
}

QString CmakeRunConfiguration::name() {
    return d->name;
}

bool CmakeRunConfiguration::haveConfigurationStep() {
    return true;
}

BuildJobPtr CmakeRunConfiguration::configure() {
    auto project = d->project.toStrongRef();
    if (!project) return nullptr;

    if (!d->buildDirectory.exists()) d->buildDirectory.mkpath(".");
    auto buildJob = BuildJobPtr(new CmakeConfigureJob(d->project, d->buildEngine, d->name, d->cmakeArgs, d->buildDirectory, d->cmakeEnv));
    connect(buildJob.data(), &BuildJob::stateChanged, this, [=](BuildJob::State state) {
        if (state == BuildJob::Successful || state == BuildJob::Failed) emit targetsChanged();
    });
    return buildJob;
}

QStringList CmakeRunConfiguration::targets() {
    auto replyIndex = ReplyIndexPtr(new ReplyIndex(d->buildDirectory));
    return replyIndex->codemodel()->targets();
}

QString CmakeRunConfiguration::recommendedTarget() {
    auto replyIndex = ReplyIndexPtr(new ReplyIndex(d->buildDirectory));
    auto codemodel = replyIndex->codemodel();
    auto targets = codemodel->targets();

    if (targets.isEmpty()) return "";
    for (auto target : targets) {
        if (codemodel->target(target)->type() == TargetObject::Executable) return target;
    }
    return targets.first();
}

bool CmakeRunConfiguration::haveBuildStep() {
    return true;
}

BuildJobPtr CmakeRunConfiguration::build(QString target) {
    auto project = d->project.toStrongRef();
    if (!project) return nullptr;

    if (target == "") target = this->recommendedTarget();

    if (!d->buildDirectory.exists()) d->buildDirectory.mkpath(".");
    return BuildJobPtr(new CmakeBuildJob(d->project, d->buildEngine, d->name, d->buildDirectory, target));
}

bool CmakeRunConfiguration::canRun(QString target) {
    auto replyIndex = ReplyIndexPtr(new ReplyIndex(d->buildDirectory));
    auto targetInfo = replyIndex->codemodel()->target(target);

    if (!targetInfo) return false;
    if (targetInfo->artifacts().isEmpty()) return false;

    return true;
}

RunJobPtr CmakeRunConfiguration::run(QString target) {
    auto replyIndex = ReplyIndexPtr(new ReplyIndex(d->buildDirectory));
    auto codeModel = replyIndex->codemodel();
    auto targetInfo = codeModel->target(target);
    auto directoryInfo = codeModel->directoryForTarget(target);

    auto* runJob = new ProcessRunJob(d->buildDirectory.absoluteFilePath(targetInfo->artifacts().first()), {}, d->buildDirectory.absoluteFilePath(directoryInfo->buildPath()));
    return RunJobPtr(runJob);
}
