#ifndef PROJECT_H
#define PROJECT_H

#include "libtheslate_global.h"
#include "lsp/languageserverprocess.h"
#include "project/buildjob.h"
#include "project/runconfiguration.h"
#include <QCoroTask>
#include <QDir>
#include <QEnableSharedFromThis>
#include <QObject>
#include <tpromise.h>

struct ProjectPrivate;
class LIBTHESLATE_EXPORT Project : public QObject,
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
        QCoro::Task<> activeRunConfigurationConfigure();
        bool canActiveRunConfigurationBuild();
        QCoro::Task<> activeRunConfigurationBuild();
        bool canActiveRunConfigurationRun();
        QCoro::Task<> activeRunConfigurationRun();
        void reloadProjectConfigurations();

        void registerRunConfigurationReloadFile(QString file);

        QList<BuildJobPtr> buildJobs();

        void addBeforeBuildEventHandler(std::function<QCoro::Task<>()> eventHandler);

        QList<LanguageServerProcess*> languageServers();
        QCoro::Task<LanguageServerProcess*> languageServerForServerName(QString languageServer);
        QCoro::Task<LanguageServerProcess*> languageServerForFileName(QString fileName);

    signals:
        void runConfigurationsUpdated();
        void buildJobAdded(BuildJobPtr buildJob);
        void targetsChanged();
        void currentTargetChanged(QString target);

    private:
        ProjectPrivate* d;
        explicit Project(QString projectDir, QObject* parent = nullptr);

        QCoro::Task<> runBeforeBuildEventHandlers();
        BuildJobPtr startBuildJob();
};

typedef QSharedPointer<Project> ProjectPtr;

#endif // PROJECT_H
