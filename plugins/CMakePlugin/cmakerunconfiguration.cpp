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
};

CmakeRunConfiguration::CmakeRunConfiguration(ProjectPtr project, CmakeBuildEngine* buildEngine, QString name, QJsonObject descriptor, QObject* parent) :
    RunConfiguration{parent} {
    d = new CmakeRunConfigurationPrivate();
    d->project = project;
    d->name = name;
    d->buildEngine = buildEngine;

    d->buildDirectory = project->projectDir(Project::BuildRootDirectory).absoluteFilePath(name.toLower().replace(" ", "_"));

    for (auto arg : descriptor.value("cmake-args").toArray()) {
        d->cmakeArgs.append(arg.toString());
    }

    connect(project.data(), &Project::runConfigurationsUpdated, this, [=]() -> QCoro::Task<> {
        if (project->activeRunConfiguration() == this) {
            // Restart clangd with this run configuration
            auto lsp = co_await project->languageServerForServerName("clangd");
            co_await lsp->startLanguageServer({
                {"compilationDatabasePath", d->buildDirectory.absolutePath()}
            });
        }
    });
}

CmakeRunConfiguration::~CmakeRunConfiguration() {
    delete d;
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
    auto buildJob = BuildJobPtr(new CmakeConfigureJob(d->project, d->buildEngine, d->name, d->cmakeArgs, d->buildDirectory));
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
