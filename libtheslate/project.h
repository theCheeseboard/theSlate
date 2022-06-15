#ifndef PROJECT_H
#define PROJECT_H

#include "project/buildjob.h"
#include "project/runconfiguration.h"
#include <QDir>
#include <QEnableSharedFromThis>
#include <QObject>
#include <tpromise.h>

struct ProjectPrivate;
class Project : public QObject,
                public QEnableSharedFromThis<Project> {
        Q_OBJECT
    public:
        ~Project();

        static QSharedPointer<Project> createProject(QString projectDir);

        enum ProjectDirectory {
            TheSlateDirectory,
            RunConfigurationCache,
            BuildRootDirectory
        };

        QDir projectDir();
        QDir projectDir(ProjectDirectory directory);

        QList<RunConfigurationPtr> runConfigurations();
        RunConfigurationPtr activeRunConfiguration();
        void setActiveRunConfiguration(RunConfigurationPtr runConfiguration);

        QStringList targets();
        QString activeTarget();
        void setActiveTarget(QString target);

        bool canActiveRunConfigurationConfigure();
        void activeRunConfigurationConfigure();
        bool canActiveRunConfigurationBuild();
        void activeRunConfigurationBuild();
        bool canActiveRunConfigurationRun();
        void activeRunConfigurationRun();
        void reloadProjectConfigurations();

        QList<BuildJobPtr> buildJobs();

        void addBeforeBuildEventHandler(std::function<tPromise<void>*()> eventHandler);

    signals:
        void runConfigurationsUpdated();
        void buildJobAdded(BuildJobPtr buildJob);
        void targetsChanged();
        void currentTargetChanged(QString target);

    private:
        ProjectPrivate* d;
        explicit Project(QString projectDir, QObject* parent = nullptr);

        tPromise<void>* runBeforeBuildEventHandlers();
        BuildJobPtr startBuildJob();
};

typedef QSharedPointer<Project> ProjectPtr;

#endif // PROJECT_H
