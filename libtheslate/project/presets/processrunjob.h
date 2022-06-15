#ifndef PROCESSRUNJOB_H
#define PROCESSRUNJOB_H

#include "../runjob.h"

struct ProcessRunJobPrivate;
class ProcessRunJob : public RunJob {
        Q_OBJECT
    public:
        explicit ProcessRunJob(QString process, QStringList args, QString cwd, QObject* parent = nullptr);
        ~ProcessRunJob();

    signals:

    private:
        ProcessRunJobPrivate* d;

        // RunJob interface
    public:
        void start();
};

#endif // PROCESSRUNJOB_H
