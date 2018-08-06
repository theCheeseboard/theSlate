#include "gitintegration.h"

#include <QSettings>
#include <QDirIterator>
#include <the-libs_global.h>

GitIntegration::GitIntegration(QDir rootDir, QObject *parent) : QObject(parent)
{
    this->rootDir = rootDir;

    QTimer* timer = new QTimer(this);
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), this, SIGNAL(reloadStatusNeeded()));
    timer->start();

    reloadStatus();
}

tPromise<QStringList>* GitIntegration::reloadStatus() {
    return new tPromise<QStringList>([=](QString& error) {
        QProcess* proc = git("rev-parse --show-toplevel");

        if (proc->exitCode() == 0) {
            TPROMISE_CALL("setRootDir", Q_ARG(QString, proc->readAll().trimmed()));
        }
        proc->deleteLater();

        proc = git("status --porcelain=1");
        proc->waitForFinished();
        QString status = proc->readAll();
        proc->deleteLater();

        return status.split('\n');
    });
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

void GitIntegration::abortMerge() {
    QProcess* proc = git("merge --abort");
    proc->waitForFinished();
    emit reloadStatusNeeded();
}

void GitIntegration::init() {
    if (needsInit()) {
        QProcess* proc = git("init");
        proc->waitForFinished();
        proc->deleteLater();
        emit reloadStatusNeeded();
    }
}

GitTask* GitIntegration::pull() {
    GitTask* task = new GitTask;
    QProcess* proc = git("pull");
    connect(proc, &QProcess::readyRead, [=] {
        task->appendToBuffer(proc->readAll());
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0) {
            emit task->finished();
        } else {
            QString output = task->buffer();
            if (output.contains("CONFLICT")) {
                emit task->failed("CONFLICT");
            } else if (output.contains("Please commit")) {
                emit task->failed("UNCLEAN");
            }
        }
        proc->deleteLater();
        task->deleteLater();
    });
    return task;
}

GitTask* GitIntegration::push() {
    GitTask* task = new GitTask;
    QProcess* proc = git("push");
    connect(proc, &QProcess::readyRead, [=] {
        task->appendToBuffer(proc->readAll());
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0) {
            emit task->finished();
        } else {
            QString output = task->buffer();
            if (output.contains("(fetch first)") || output.contains("(non-fast-forward)")) {
                emit task->failed("UPDATE");
            }
        }
        proc->deleteLater();
        task->deleteLater();
    });
    return task;
}

QString GitIntegration::commit(QString message) {
    QProcess* proc = git("commit -m \"" + message + "\"");
    proc->waitForFinished();
    proc->deleteLater();

    proc = git("rev-parse --short HEAD");
    proc->waitForFinished();
    QString retval = proc->readAll().trimmed();
    proc->deleteLater();

    emit reloadStatusNeeded();
    return retval;
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
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->setWorkingDirectory(rootDir.path());
    proc->start(gitInstance + " " + args);

    return proc;
}

void GitIntegration::setRootDir(QDir rootDir) {
    this->rootDir = rootDir;
}

GitTask::GitTask(QObject* parent) : QObject(parent) {

}

void GitTask::appendToBuffer(QByteArray append) {
    buf.append(append);

    QStringList lines = QString(buf).split("\n", QString::SkipEmptyParts);
    if (lines.length() > 0) {
        emit output(lines.last());
    }
}

QByteArray GitTask::buffer() {
    return buf;
}
