#ifndef BUILDENGINEMANAGER_H
#define BUILDENGINEMANAGER_H

#include "project.h"
#include "runconfiguration.h"
#include "libtheslate_global.h"
#include <QObject>

class BuildEngine;
struct BuildEngineManagerPrivate;
class LIBTHESLATE_EXPORT BuildEngineManager : public QObject {
        Q_OBJECT
    public:
        explicit BuildEngineManager(QObject* parent = nullptr);
        ~BuildEngineManager();

        void registerBuildEngine(BuildEngine* buildEngine);
        QList<RunConfigurationPtr> discoverRunConfigurations(ProjectPtr project);

    signals:

    private:
        BuildEngineManagerPrivate* d;
};

#endif // BUILDENGINEMANAGER_H
