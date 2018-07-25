#include "gitintegration.h"

#include <QSettings>

GitIntegration::GitIntegration(QDir rootDir, QObject *parent) : QObject(parent)
{
    this->rootDir = rootDir;
    proc = new QProcess();
    proc->setWorkingDirectory(rootDir.path());

    proc->start("git rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString newrootDir = proc->readAll().trimmed();
        proc->setWorkingDirectory(newrootDir);
    }

    QFileSystemWatcher* watcher = new QFileSystemWatcher;
    watcher->addPath(rootDir.path());
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
    connect(watcher, SIGNAL(fileChanged(QString)), this, SIGNAL(reloadStatusNeeded()));
}

QStringList GitIntegration::reloadStatus() {
    proc->start("git rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString newrootDir = proc->readAll().trimmed();
        proc->setWorkingDirectory(newrootDir);
    }

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
    proc->start("git rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() != 0) {
        return true;
    } else {
        return false;
    }
}

void GitIntegration::init() {
    if (needsInit()) {
        proc->start("git init");
        proc->waitForFinished();
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
                gitExecutables.append(gitSearch.value("InstallLocation").toString().append("bin\\git.exe"));
            }
        }
        return gitExecutables;
    #else
        //look in the PATH
        return QStringList() << "/usr/bin/git";
    #endif
}
