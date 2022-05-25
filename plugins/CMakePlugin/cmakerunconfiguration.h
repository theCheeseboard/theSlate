#ifndef CMAKERUNCONFIGURATION_H
#define CMAKERUNCONFIGURATION_H

#include "project.h"
#include "project/runconfiguration.h"

class CmakeBuildEngine;
struct CmakeRunConfigurationPrivate;
class CmakeRunConfiguration : public RunConfiguration {
        Q_OBJECT
    public:
        explicit CmakeRunConfiguration(ProjectPtr project, CmakeBuildEngine* buildEngine, QString name, QJsonObject descriptor, QObject* parent = nullptr);
        ~CmakeRunConfiguration();

    signals:

    private:
        CmakeRunConfigurationPrivate* d;

        // RunConfiguration interface
public:
        QString name();

        // RunConfiguration interface
public:
        bool haveConfigurationStep();
        BuildJobPtr configure();
        bool haveBuildStep();
        BuildJobPtr build();
};

#endif // CMAKERUNCONFIGURATION_H
