#include "project.h"

#include "project/buildenginemanager.h"
#include "project/runconfiguration.h"
#include "statemanager.h"
#include <QCoroTimer>
#include <QTimer>

struct ProjectPrivate {
        QList<RunConfigurationPtr> runConfigurations;
        RunConfigurationPtr activeRunConfiguration;
        QString activeTarget;
        QString projectDir;

        QList<BuildJobPtr> buildJobs;

        QMutex languageServerMutex;
        QMap<QString, LanguageServerProcess*> languageServers;

        QFileSystemWatcher watcher;

        QList<std::function<QCoro::Task<>()>> beforeBuildEventHandlers;
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

    connect(&d->watcher, &QFileSystemWatcher::fileChanged, this, &Project::reloadProjectConfigurations);
}

QCoro::Task<> Project::runBeforeBuildEventHandlers() {
    for (auto handler : d->beforeBuildEventHandlers) {
        co_await handler();
    }
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

QCoro::Task<> Project::activeRunConfigurationConfigure() {
    if (!d->activeRunConfiguration) co_return;

    co_await this->runBeforeBuildEventHandlers();

    auto buildJob = d->activeRunConfiguration->configure();
    if (!buildJob) co_return;
    d->buildJobs.append(buildJob);
    emit buildJobAdded(buildJob);

    buildJob->start();
}

bool Project::canActiveRunConfigurationBuild() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->haveBuildStep();
}

QCoro::Task<> Project::activeRunConfigurationBuild() {
    if (!d->activeRunConfiguration) co_return;

    co_await this->runBeforeBuildEventHandlers();
    this->startBuildJob();
}

bool Project::canActiveRunConfigurationRun() {
    if (!d->activeRunConfiguration) return false;
    return d->activeRunConfiguration->canRun(d->activeTarget);
}

QCoro::Task<> Project::activeRunConfigurationRun() {
    if (!d->activeRunConfiguration) co_return;

    co_await this->runBeforeBuildEventHandlers();
    auto buildJob = this->startBuildJob();
    connect(buildJob.data(), &BuildJob::stateChanged, this, [=](BuildJob::State state) {
        if (state == BuildJob::Successful) {
            // TODO: Register this run job so we can stop it, etc.
            auto runJob = d->activeRunConfiguration->run(d->activeTarget);
            runJob->start();
        }
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

void Project::registerRunConfigurationReloadFile(QString file)
{
    d->watcher.addPath(file);
}

QList<BuildJobPtr> Project::buildJobs() {
    return d->buildJobs;
}

void Project::addBeforeBuildEventHandler(std::function<QCoro::Task<>()> eventHandler) {
    d->beforeBuildEventHandlers.append(eventHandler);
}

QList<LanguageServerProcess*> Project::languageServers() {
    return d->languageServers.values();
}

QCoro::Task<LanguageServerProcess*> Project::languageServerForServerName(QString languageServer) {
    if (d->languageServers.contains(languageServer)) co_return d->languageServers.value(languageServer);

    while (!d->languageServerMutex.tryLock(0)) {
        QTimer timer;
        timer.setInterval(100);
        timer.start();
        co_await timer;
    }

    if (d->languageServers.contains(languageServer)) {
        d->languageServerMutex.unlock();
        co_return d->languageServers.value(languageServer);
    }

    LanguageServerProcess* lsp = new LanguageServerProcess(languageServer, this);
    co_await lsp->startLanguageServer();
    lsp->addWorkspaceFolder(QUrl::fromLocalFile(this->projectDir().absolutePath()), "project");
    d->languageServers.insert(languageServer, lsp);

    d->languageServerMutex.unlock();
    co_return lsp;
}

QCoro::Task<LanguageServerProcess*> Project::languageServerForFileName(QString fileName) {
    QString recommendedServer = LanguageServerProcess::serverTypeForFileName(fileName);
    if (recommendedServer == "") co_return nullptr;
    co_return co_await languageServerForServerName(recommendedServer);
}
