#include "gitintegration.h"

#include <QSettings>
#include <QDirIterator>
#include <the-libs_global.h>

GitIntegration::GitIntegration(QString rootDir, QObject *parent) : QObject(parent)
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
        proc->waitForFinished();

        if (proc->exitCode() == 0) {
            QString rootDir = proc->readAll().trimmed();
            if (this->rootDir != rootDir) {
                this->setRootDir(rootDir);
            }
            proc->deleteLater();

            proc = git("status --porcelain=1");
            proc->waitForFinished();
            QString status = proc->readAll();
            proc->deleteLater();

            return status.split('\n');
        } else {
            proc->deleteLater();
            return QStringList();
        }
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
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
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
    proc->setWorkingDirectory(rootDir);
    proc->start(gitInstance + " " + args);

    return proc;
}

void GitIntegration::setRootDir(QString rootDir) {
    QMutexLocker locker(&instanceLocker);
    this->rootDir.clear();
    this->rootDir.append(rootDir);
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

GitIntegration::CommitPointer GitIntegration::getCommit(QString hash, bool populate, bool iterateParents) {
    CommitPointer commit;
    if (knownCommits.contains(hash)) {
        commit = knownCommits.value(hash);
        if (!populate || commit->populated) {
            return commit;
        }
    }
    QProcess* show = git("show --format=\"format:%H;%an;%ae;%cn;%ce;%ct;%P;%s\" --quiet " + hash);
    show->waitForFinished();

    if (show->exitCode() != 0) {
        return CommitPointer();
    }

    QStringList parts = QString(show->readLine()).trimmed().split(";");

    if (knownCommits.contains(parts.at(0))) {
        commit = knownCommits.value(parts.at(0));
        if (!populate || commit->populated) {
            return commit;
        }
    } else {
        commit = CommitPointer(new Commit());
    }

    commit->hash = parts.at(0);

    if (populate) {
        commit->author = parts.at(1);
        commit->authorEmail = parts.at(2);
        commit->committer = parts.at(3);
        commit->committerEmail = parts.at(4);
        commit->commitDate = QDateTime::fromSecsSinceEpoch(parts.at(5).toInt());

        if (iterateParents) {
            QStringList parentIds = parts.at(6).split(" ");
            for (QString parent : parentIds) {
                commit->parents.append(getCommit(parent, false, true));
            }
            commit->parentsKnown = true;
        }

        commit->message = parts.at(7);
        commit->populated = true;

        emit commitInformationAvailable(commit->hash);
    }

    knownCommits.insert(commit->hash, commit);

    return commit;
}

tPromise<GitIntegration::CommitList>* GitIntegration::commits(QString branch) {
    return new tPromise<CommitList>([=](QString& error) -> CommitList {
        CommitList commits;

        QMutex* contMutex = new QMutex();
        bool* cont = new bool(true);

        QProcess* revlist = git("rev-list " + branch);
        revlist->waitForFinished();

        connect(this, &GitIntegration::commitsChanged, [=] {
            contMutex->lock();
            *cont = false;
            contMutex->unlock();
        });

        contMutex->lock();
        while (revlist->canReadLine() && *cont) {
            contMutex->unlock();
            commits.append(getCommit(revlist->readLine().trimmed(), false, false));
            contMutex->lock();
        }
        contMutex->unlock();

        revlist->deleteLater();

        if (!*cont) {
            error = "Cancelled";
        }
        delete cont;
        return commits;
    });
}

bool GitIntegration::setNewRootDir(QString rootDir) {
    QString oldRootDir = this->rootDir;
    this->setRootDir(rootDir);

    QProcess* proc = git("rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString rootDir = proc->readAll().trimmed();
        this->setRootDir(rootDir);

        bool retval = false;
        if (oldRootDir != rootDir) {
            retval = true;

            knownCommits.clear();
            emit commitsChanged();
        }
        proc->deleteLater();

        return retval;
    } else {
        return true;
    }
}

