#ifndef RUNCONFIGURATION_H
#define RUNCONFIGURATION_H

#include "buildjob.h"
#include <QObject>

class RunConfiguration : public QObject {
        Q_OBJECT
    public:
        explicit RunConfiguration(QObject* parent = nullptr);

        virtual QString name() = 0;

        virtual bool haveConfigurationStep() = 0;
        virtual BuildJobPtr configure() = 0;

        virtual bool haveBuildStep() = 0;
        virtual BuildJobPtr build() = 0;

    signals:
};
typedef QSharedPointer<RunConfiguration> RunConfigurationPtr;

#endif // RUNCONFIGURATION_H
