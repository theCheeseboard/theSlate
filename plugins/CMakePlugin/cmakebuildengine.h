#ifndef CMAKEBUILDENGINE_H
#define CMAKEBUILDENGINE_H

#include "project/buildengine.h"

struct CmakeBuildEnginePrivate;
class CmakeBuildEngine : public BuildEngine {
        Q_OBJECT
    public:
        explicit CmakeBuildEngine(QObject* parent = nullptr);
        ~CmakeBuildEngine();

        QString cmakeExecutable();

    signals:

    private:
        CmakeBuildEnginePrivate* d;
        QList<RunConfigurationPtr> runConfigurationsFromConfigurationFile(QJsonObject configurationFile, ProjectPtr project);

        // BuildEngine interface
    public:
        QList<RunConfigurationPtr> discoverRunConfigurations(ProjectPtr project);
};

#endif // CMAKEBUILDENGINE_H
