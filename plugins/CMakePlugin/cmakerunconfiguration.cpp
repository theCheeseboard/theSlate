#include "cmakerunconfiguration.h"

#include <QJsonArray>
#include <QJsonObject>

#include "buildjobs/cmakebuildjob.h"
#include "buildjobs/cmakeconfigurejob.h"

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
    ProjectPtr project = d->project.toStrongRef();
    if (!project) return nullptr;

    if (!d->buildDirectory.exists()) d->buildDirectory.mkpath(".");
    return BuildJobPtr(new CmakeConfigureJob(d->project, d->buildEngine, d->name, d->cmakeArgs, d->buildDirectory));
}

bool CmakeRunConfiguration::haveBuildStep() {
    return true;
}

BuildJobPtr CmakeRunConfiguration::build() {
    ProjectPtr project = d->project.toStrongRef();
    if (!project) return nullptr;

    if (!d->buildDirectory.exists()) d->buildDirectory.mkpath(".");
    return BuildJobPtr(new CmakeBuildJob(d->project, d->buildEngine, d->name, d->buildDirectory));
}
