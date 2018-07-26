#include "gitintegration.h"

#include <QSettings>

#ifndef Q_OS_WIN
#include <the-libs_global.h>
#endif

GitIntegration::GitIntegration(QDir rootDir, QObject *parent) : QObject(parent)
{
    this->rootDir = rootDir;

    QProcess* proc = git("rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString newrootDir = proc->readAll().trimmed();
        proc->setWorkingDirectory(newrootDir);
    }
    proc->deleteLater();

    QFileSystemWatcher* watcher = new QFileSystemWatcher;
    watcher->addPath(rootDir.path());
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
    connect(watcher, SIGNAL(fileChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
}

QStringList GitIntegration::reloadStatus() {
    QProcess* proc = git("rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString newrootDir = proc->readAll().trimmed();
        proc->setWorkingDirectory(newrootDir);
    }
    proc->deleteLater();

    proc = git("status --porcelain=1");
    proc->waitForFinished();
    QString status = proc->readAll();
    proc->deleteLater();

    return status.split('\n');
}

void GitIntegration::add(QString file) {
    QProcess* proc = git("add -- " + file);
    proc->waitForFinished();
    proc->deleteLater();
    emit reloadStatusNeeded();
}

void GitIntegration::rm(QString file, bool cache) {
    QProcess* proc;
    if (cache) {
        proc = git("rm --cached -- " + file);
    } else {
        proc = git("rm -- " + file);
    }
    proc->waitForFinished();
    proc->deleteLater();
    emit reloadStatusNeeded();
}

void GitIntegration::unstage(QString file) {
    QProcess* proc = git("reset HEAD " + file);
    proc->waitForFinished();
    proc->deleteLater();
    emit reloadStatusNeeded();
}

bool GitIntegration::needsInit() {
    QProcess* proc = git("rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() != 0) {
        proc->deleteLater();
        return true;
    } else {
        proc->deleteLater();
        return false;
    }
}

void GitIntegration::init() {
    if (needsInit()) {
        QProcess* proc = git("git init");
        proc->waitForFinished();
        proc->deleteLater();
        emit reloadStatusNeeded();
    }
}

QStringList GitIntegration::findGit() {
    #ifdef Q_OS_WIN
        //Search the registry for Git
        QStringList gitExecutables;

        QStringList hives;
        hives << "HKEY_LOCAL_MACHINE" << "HKEY_CURRENT_USER";
        for (QString hive : hives) {
            QSettings gitSearch(QString("%1\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Git_is1").arg(hive), QSettings::NativeFormat);
            if (gitSearch.contains("InstallLocation")) {
                gitExecutables.append("\"" + gitSearch.value("InstallLocation").toString().append("bin\\git.exe") + "\"");
            }
        }
        return gitExecutables;
    #else
        //Look in the PATH
        return theLibsGlobal::searchInPath("git");
    #endif
}

QProcess* GitIntegration::git(QString args) {
    if (gitInstance == "") {
        QStringList instances = findGit();
        if (instances.count() > 0) {
            gitInstance = instances.first();
        }
    }

    QProcess* proc = new QProcess();
    proc->setWorkingDirectory(rootDir.path());

    proc->start(gitInstance + " " + args);

    return proc;
}
