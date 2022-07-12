#ifndef RUNCONFIGURATION_H
#define RUNCONFIGURATION_H

#include "buildjob.h"
#include "runjob.h"
#include "libtheslate_global.h"
#include <QObject>

class LIBTHESLATE_EXPORT RunConfiguration : public QObject {
        Q_OBJECT
    public:
        explicit RunConfiguration(QObject* parent = nullptr);

        virtual QString name() = 0;

        virtual bool haveConfigurationStep() = 0;
        virtual BuildJobPtr configure() = 0;

        virtual QStringList targets() = 0;
        virtual QString recommendedTarget() = 0;

        virtual bool haveBuildStep() = 0;
        virtual BuildJobPtr build(QString target = "") = 0;

        virtual bool canRun(QString target = "") = 0;
        virtual RunJobPtr run(QString target = "") = 0;

    signals:
        void targetsChanged();
};
typedef QSharedPointer<RunConfiguration> RunConfigurationPtr;

#endif // RUNCONFIGURATION_H
