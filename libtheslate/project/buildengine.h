#ifndef BUILDENGINE_H
#define BUILDENGINE_H

#include "project.h"
#include "runconfiguration.h"
#include <QObject>

class BuildEngine : public QObject {
        Q_OBJECT
    public:
        explicit BuildEngine(QObject* parent = nullptr);

        virtual QList<RunConfigurationPtr> discoverRunConfigurations(ProjectPtr project) = 0;

    signals:
};

#endif // BUILDENGINE_H
