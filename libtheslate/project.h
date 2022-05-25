#ifndef PROJECT_H
#define PROJECT_H

#include "project/buildjob.h"
#include "project/runconfiguration.h"
#include <QDir>
#include <QEnableSharedFromThis>
#include <QObject>

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

        bool canActiveRunConfigurationConfigure();
        void activeRunConfigurationConfigure();
        bool canActiveRunConfigurationBuild();
        void activeRunConfigurationBuild();
        void reloadProjectConfigurations();

        QList<BuildJobPtr> buildJobs();

    signals:
        void runConfigurationsUpdated();
        void buildJobAdded(BuildJobPtr buildJob);

    private:
        ProjectPrivate* d;
        explicit Project(QString projectDir, QObject* parent = nullptr);
};

typedef QSharedPointer<Project> ProjectPtr;

#endif // PROJECT_H
