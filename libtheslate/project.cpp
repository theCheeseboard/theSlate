#include "project.h"

#include "project/buildenginemanager.h"
#include "project/runconfiguration.h"
#include "statemanager.h"
#include <QTimer>

struct ProjectPrivate {
        QList<RunConfigurationPtr> runConfigurations;
        RunConfigurationPtr activeRunConfiguration;
        QString projectDir;

        QList<BuildJobPtr> buildJobs;
};

Project::Project(QString projectDir, QObject* parent) :
    QObject{parent} {
    d = new ProjectPrivate();
    d->projectDir = projectDir;

    QTimer::singleShot(0, this, &Project::reloadProjectConfigurations);
}

Project::~Project() {
    delete d;
}

QSharedPointer<Project> Project::createProject(QString projectDir) {
    return ProjectPtr(new Project(projectDir));
}

QDir Project::projectDir() {
    return d->projectDir;
}

QDir Project::projectDir(ProjectDirectory directory) {
    QDir dir;
    switch (directory) {
        case Project::TheSlateDirectory:
            dir = this->projectDir().absoluteFilePath(".theslate");
            break;
        case Project::RunConfigurationCache:
            dir = this->projectDir(TheSlateDirectory).absoluteFilePath("runconfig");
            break;
        case Project::BuildRootDirectory:
            dir = this->projectDir().absoluteFilePath("theslate-builds");
            break;
    }

    if (!dir.exists()) dir.mkpath(".");
    return dir;
}

QList<RunConfigurationPtr> Project::runConfigurations() {
    return d->runConfigurations;
}

RunConfigurationPtr Project::activeRunConfiguration() {
    return d->activeRunConfiguration;
}

void Project::setActiveRunConfiguration(RunConfigurationPtr runConfiguration) {
    d->activeRunConfiguration = runConfiguration;
    emit runConfigurationsUpdated();
}

bool Project::canActiveRunConfigurationConfigure() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->haveConfigurationStep();
}

void Project::activeRunConfigurationConfigure() {
    if (!d->activeRunConfiguration) return;

    auto buildJob = d->activeRunConfiguration->configure();
    if (!buildJob) return;
    d->buildJobs.append(buildJob);
    emit buildJobAdded(buildJob);

    buildJob->start();
}

bool Project::canActiveRunConfigurationBuild() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->haveBuildStep();
}

void Project::activeRunConfigurationBuild() {
    if (!d->activeRunConfiguration) return;

    auto buildJob = d->activeRunConfiguration->build();
    if (!buildJob) return;
    d->buildJobs.append(buildJob);
    emit buildJobAdded(buildJob);

    buildJob->start();
}

void Project::reloadProjectConfigurations() {
    QString activeRunConfigurationName = "";
    if (d->activeRunConfiguration) activeRunConfigurationName = d->activeRunConfiguration->name();

    d->runConfigurations = StateManager::buildEngine()->discoverRunConfigurations(sharedFromThis());

    if (d->runConfigurations.isEmpty()) {
        d->activeRunConfiguration.clear();
    } else {
        d->activeRunConfiguration = d->runConfigurations.first();

        for (auto runConfiguration : d->runConfigurations) {
            if (runConfiguration->name() == activeRunConfigurationName) {
                d->activeRunConfiguration = runConfiguration;
            }
        }
    }

    emit runConfigurationsUpdated();
}

QList<BuildJobPtr> Project::buildJobs() {
    return d->buildJobs;
}
