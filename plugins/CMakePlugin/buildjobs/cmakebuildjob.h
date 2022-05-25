#ifndef CMAKEBUILDJOB_H
#define CMAKEBUILDJOB_H

#include "project.h"
#include "project/buildjob.h"

class CmakeBuildEngine;
struct CmakeBuildJobPrivate;
class CmakeBuildJob : public BuildJob {
        Q_OBJECT
    public:
        explicit CmakeBuildJob(ProjectPtr project, CmakeBuildEngine* buildEngine, QString configurationName, QDir buildDirectory, QObject* parent = nullptr);
        ~CmakeBuildJob();

    signals:

    private:
        CmakeBuildJobPrivate* d;

        void processOutput(QString output);

        // BuildJob interface
    public:
        void start();
};

#endif // CMAKEBUILDJOB_H
