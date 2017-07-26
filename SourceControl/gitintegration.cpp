#include "gitintegration.h"

GitIntegration::GitIntegration(QDir rootDir, QObject *parent) : QObject(parent)
{
    this->rootDir = rootDir;
    proc = new QProcess();
    proc->setWorkingDirectory(rootDir.path());

    QFileSystemWatcher* watcher = new QFileSystemWatcher;
    watcher->addPath(rootDir.path());
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
    connect(watcher, SIGNAL(fileChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
}

QStringList GitIntegration::reloadStatus() {
    proc->start("git status --porcelain=1");
    proc->waitForFinished();

    return QString(proc->readAll()).split('\n');
}

void GitIntegration::add(QString file) {
    proc->start("git add -- " + file);
    proc->waitForFinished();
    emit reloadStatusNeeded();
}

void GitIntegration::rm(QString file, bool cache) {
    if (cache) {
        proc->start("git rm --cached -- " + file);
    } else {
        proc->start("git rm -- " + file);
    }
    proc->waitForFinished();
    emit reloadStatusNeeded();
}

void GitIntegration::unstage(QString file) {
    proc->start("git reset HEAD " + file);
    proc->waitForFinished();
    emit reloadStatusNeeded();
}

bool GitIntegration::needsInit() {
    if (QDir(rootDir.path() + "/.git").exists()) {
        return false;
    } else {
        return true;
    }
}

void GitIntegration::init() {
    if (needsInit()) {
        proc->start("git init");
        proc->waitForFinished();
        emit reloadStatusNeeded();
    }
}
