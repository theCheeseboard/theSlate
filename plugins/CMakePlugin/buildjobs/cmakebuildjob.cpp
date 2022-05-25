#include "cmakebuildjob.h"

#include "cmakebuildengine.h"
#include <QProcess>
#include <QRegularExpression>

struct CmakeBuildJobPrivate {
        ProjectPtr project;
        QDir buildDirectory;
        QString name;
        CmakeBuildEngine* buildEngine;

        static QRegularExpression cmakeOutputRegex;
};

auto CmakeBuildJobPrivate::cmakeOutputRegex = QRegularExpression("\\[(\\d+)/(\\d+)] (.+)");

CmakeBuildJob::CmakeBuildJob(ProjectPtr project, CmakeBuildEngine* buildEngine, QString configurationName, QDir buildDirectory, QObject* parent) :
    BuildJob{parent} {
    d = new CmakeBuildJobPrivate();
    d->project = project;
    d->buildEngine = buildEngine;
    d->buildDirectory = buildDirectory;
    d->name = configurationName;

    this->setTitle(tr("Build %1").arg(QLocale().quoteString(configurationName)));
    this->setDescription(tr("Building Project..."));
}

CmakeBuildJob::~CmakeBuildJob() {
    delete d;
}

void CmakeBuildJob::processOutput(QString output) {
    appendToBuildLog(output);

    for (const auto& line : output.trimmed().split("\n")) {
        auto match = d->cmakeOutputRegex.match(line);
        if (match.hasMatch()) {
            this->setStep(match.captured(1).toInt());
            this->setMaxStep(match.captured(2).toInt());
            this->setProgress(match.captured(1).toInt());
            this->setMaxProgress(match.captured(2).toInt());
            this->setDescription(match.captured(3));
        }
    }
}

void CmakeBuildJob::start() {
    QStringList args = {
        "--build",
        d->buildDirectory.path()};

    auto* cmakeProc = new QProcess();
    connect(cmakeProc, &QProcess::readyReadStandardOutput, this, [=] {
        processOutput(cmakeProc->readAllStandardOutput());
    });
    connect(cmakeProc, &QProcess::readyReadStandardError, this, [=] {
        processOutput(cmakeProc->readAllStandardError());
    });
    connect(cmakeProc, &QProcess::finished, this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
            this->setState(Successful);
            this->setDescription(tr("Build Complete"));
        } else {
            this->setState(Failed);
            this->setDescription(tr("Build Failed"));
        }
    });
    cmakeProc->start(d->buildEngine->cmakeExecutable(), args);
}
