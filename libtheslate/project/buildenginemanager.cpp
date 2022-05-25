#include "buildenginemanager.h"

#include "buildengine.h"
#include <QSharedPointer>

struct BuildEngineManagerPrivate {
        QList<BuildEngine*> buildEngines;
};

BuildEngineManager::BuildEngineManager(QObject* parent) :
    QObject{parent} {
    d = new BuildEngineManagerPrivate();
}

BuildEngineManager::~BuildEngineManager() {
    delete d;
}

void BuildEngineManager::registerBuildEngine(BuildEngine* buildEngine) {
    d->buildEngines.append(buildEngine);
}

QList<RunConfigurationPtr> BuildEngineManager::discoverRunConfigurations(ProjectPtr project) {
    QList<RunConfigurationPtr> runConfigurations;
    for (BuildEngine* engine : d->buildEngines) {
        runConfigurations.append(engine->discoverRunConfigurations(project));
    }
    return runConfigurations;
}
