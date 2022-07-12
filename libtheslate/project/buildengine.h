#ifndef BUILDENGINE_H
#define BUILDENGINE_H

#include "project.h"
#include "runconfiguration.h"
#include "libtheslate_global.h"
#include <QObject>

class LIBTHESLATE_EXPORT BuildEngine : public QObject {
        Q_OBJECT
    public:
        explicit BuildEngine(QObject* parent = nullptr);

        virtual QList<RunConfigurationPtr> discoverRunConfigurations(ProjectPtr project) = 0;

    signals:
};

#endif // BUILDENGINE_H
