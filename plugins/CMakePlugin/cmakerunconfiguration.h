#ifndef CMAKERUNCONFIGURATION_H
#define CMAKERUNCONFIGURATION_H

#include "project.h"
#include "project/runconfiguration.h"
#include <QCoroGenerator>

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

        QCoro::Generator<QDir> glob(QDir dir, QStringList possibilities);

        // RunConfiguration interface
    public:
        QString name();

        // RunConfiguration interface
    public:
        bool haveConfigurationStep();
        BuildJobPtr configure();
        QStringList targets();
        QString recommendedTarget();
        bool haveBuildStep();
        BuildJobPtr build(QString target);
        bool canRun(QString target);
        RunJobPtr run(QString target);
};

#endif // CMAKERUNCONFIGURATION_H
