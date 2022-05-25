#include "cmakeconfigurejob.h"

#include "cmakebuildengine.h"
#include <QProcess>

struct CmakeConfigureJobPrivate {
        ProjectPtr project;
        QStringList cmakeArgs;
        QDir buildDirectory;
        QString name;
        CmakeBuildEngine* buildEngine;
};

CmakeConfigureJob::CmakeConfigureJob(ProjectPtr project, CmakeBuildEngine* buildEngine, QString configurationName, QStringList cmakeArgs, QDir buildDirectory, QObject* parent) :
    BuildJob{parent} {
    d = new CmakeConfigureJobPrivate();
    d->project = project;
    d->buildEngine = buildEngine;
    d->cmakeArgs = cmakeArgs;
    d->buildDirectory = buildDirectory;
    d->name = configurationName;

    this->setTitle(tr("Configure %1").arg(QLocale().quoteString(configurationName)));
    this->setDescription(tr("Configuring CMake..."));
}

CmakeConfigureJob::~CmakeConfigureJob() {
    delete d;
}

void CmakeConfigureJob::start() {
    QStringList args = {
        "-S",
        d->project->projectDir().path(),
        "-B",
        d->buildDirectory.path()};
    args.append(d->cmakeArgs);

    auto* cmakeProc = new QProcess();
    connect(cmakeProc, &QProcess::readyReadStandardOutput, this, [=] {
        QString output = cmakeProc->readAllStandardOutput();
        this->setDescription(output.trimmed().split("\n").last());
        appendToBuildLog(output);
    });
    connect(cmakeProc, &QProcess::readyReadStandardError, this, [=] {
        QString output = cmakeProc->readAllStandardError();
        this->setDescription(output.trimmed().split("\n").last());
        appendToBuildLog(output);
    });
    connect(cmakeProc, &QProcess::finished, this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
            this->setState(Successful);
            this->setDescription(tr("CMake Configuration Complete"));
        } else {
            this->setState(Failed);
            this->setDescription(tr("CMake Configuration Failed"));
        }
    });
    cmakeProc->start(d->buildEngine->cmakeExecutable(), args);
}
