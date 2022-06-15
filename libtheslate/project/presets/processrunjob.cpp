#include "processrunjob.h"

#include <QProcess>

struct ProcessRunJobPrivate {
        QProcess* proc;
};

ProcessRunJob::ProcessRunJob(QString process, QStringList args, QString cwd, QObject* parent) :
    RunJob{parent} {
    d = new ProcessRunJobPrivate();
    d->proc = new QProcess();
    d->proc->setProgram(process);
    d->proc->setArguments(args);
    d->proc->setWorkingDirectory(cwd);
}

ProcessRunJob::~ProcessRunJob() {
    delete d;
}

void ProcessRunJob::start() {
    d->proc->start();
}
