#ifndef CMAKECONFIGUREJOB_H
#define CMAKECONFIGUREJOB_H

#include "project.h"
#include "project/buildjob.h"

class CmakeBuildEngine;
struct CmakeConfigureJobPrivate;
class CmakeConfigureJob : public BuildJob {
        Q_OBJECT
    public:
        explicit CmakeConfigureJob(ProjectPtr project, CmakeBuildEngine* buildEngine, QString configurationName, QStringList cmakeArgs, QDir buildDirectory, QObject* parent = nullptr);
        ~CmakeConfigureJob();

    signals:

    private:
        CmakeConfigureJobPrivate* d;

        // BuildJob interface
    public:
        void start();
};

#endif // CMAKECONFIGUREJOB_H
