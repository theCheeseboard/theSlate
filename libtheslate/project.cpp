#include "project.h"

#include "project/buildenginemanager.h"
#include "project/runconfiguration.h"
#include "statemanager.h"
#include <QTimer>

struct ProjectPrivate {
        QList<RunConfigurationPtr> runConfigurations;
        RunConfigurationPtr activeRunConfiguration;
        QString activeTarget;
        QString projectDir;

        QList<BuildJobPtr> buildJobs;

        QList<std::function<tPromise<void>*()>> beforeBuildEventHandlers;
};

Project::Project(QString projectDir, QObject* parent) :
    QObject{parent} {
    d = new ProjectPrivate();
    d->projectDir = projectDir;

    QTimer::singleShot(0, this, &Project::reloadProjectConfigurations);

    connect(this, &Project::runConfigurationsUpdated, this, [=] {
        emit targetsChanged();

        if (!d->activeRunConfiguration) {
            setActiveTarget("");
            return;
        }

        auto targets = d->activeRunConfiguration->targets();
        if (!targets.contains(d->activeTarget)) {
            setActiveTarget(d->activeRunConfiguration->recommendedTarget());
            return;
        }
    });
}

tPromise<void>* Project::runBeforeBuildEventHandlers() {
    return TPROMISE_CREATE_NEW_THREAD(void, {
        for (auto handler : d->beforeBuildEventHandlers) {
            auto results = handler()->await();
            if (!results.error.isEmpty()) {
                rej(results.error);
                return;
            }
        }

        res();
    });
}

BuildJobPtr Project::startBuildJob() {
    auto buildJob = d->activeRunConfiguration->build(d->activeTarget);
    if (!buildJob) return BuildJobPtr();
    d->buildJobs.append(buildJob);
    emit buildJobAdded(buildJob);

    buildJob->start();
    return buildJob;
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
    if (d->activeRunConfiguration) {
        d->activeRunConfiguration->disconnect(this);
    }
    d->activeRunConfiguration = runConfiguration;
    if (d->activeRunConfiguration) {
        connect(d->activeRunConfiguration.data(), &RunConfiguration::targetsChanged, this, &Project::targetsChanged);
    }
    emit runConfigurationsUpdated();
}

QStringList Project::targets() {
    if (!d->activeRunConfiguration) return {};
    return d->activeRunConfiguration->targets();
}

QString Project::activeTarget() {
    return d->activeTarget;
}

void Project::setActiveTarget(QString target) {
    d->activeTarget = target;
    emit currentTargetChanged(target);
}

bool Project::canActiveRunConfigurationConfigure() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->haveConfigurationStep();
}

void Project::activeRunConfigurationConfigure() {
    if (!d->activeRunConfiguration) return;

    this->runBeforeBuildEventHandlers()->then([=] {
        auto buildJob = d->activeRunConfiguration->configure();
        if (!buildJob) return;
        d->buildJobs.append(buildJob);
        emit buildJobAdded(buildJob);

        buildJob->start();
    });
}

bool Project::canActiveRunConfigurationBuild() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->haveBuildStep();
}

void Project::activeRunConfigurationBuild() {
    if (!d->activeRunConfiguration) return;

    this->runBeforeBuildEventHandlers()->then([=] {
        this->startBuildJob();
    });
}

bool Project::canActiveRunConfigurationRun() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->canRun(d->activeTarget);
}

void Project::activeRunConfigurationRun() {
    if (!d->activeRunConfiguration) return;

    this->runBeforeBuildEventHandlers()->then([=] {
        auto buildJob = this->startBuildJob();
        connect(buildJob.data(), &BuildJob::stateChanged, this, [=](BuildJob::State state) {
            if (state == BuildJob::Successful) {
                // TODO: Register this run job so we can stop it, etc.
                auto runJob = d->activeRunConfiguration->run(d->activeTarget);
                runJob->start();
            }
        });
    });
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

void Project::addBeforeBuildEventHandler(std::function<tPromise<void>*()> eventHandler) {
    d->beforeBuildEventHandlers.append(eventHandler);
}