#include "gitintegration.h"

#include <QSettings>
#include <QDirIterator>
#include <the-libs_global.h>

struct GitIntegrationPrivate {
    QString rootDir;
    QString gitInstance;

    QString headCommit;
    GitIntegration::BranchPointer currentBranch;
    int pendingIn = 0;
    int pendingOut = 0;

    QMap<QString, GitIntegration::CommitPointer> knownCommits;
    QMap<QString, GitIntegration::BranchPointer> knownBranches;
};

GitIntegration::GitIntegration(QString rootDir, QObject *parent) : QObject(parent)
{
    d = new GitIntegrationPrivate();
    d->rootDir = rootDir;

    watcher = new QFileSystemWatcher();
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &GitIntegration::watcherChanged);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &GitIntegration::watcherChanged);

    connect(this, &GitIntegration::headCommitChanged, [=] {
        //Clear all the commit pointers
        for (GitIntegration::CommitPointer commit : d->knownCommits.values()) {
            commit->pointersKnown = false;
        }
    });

    QTimer* timer = new QTimer(this);
    timer->setInterval(30000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=] {
        fetch()->then([=] {
            //Restart the timer after fetching is done
            timer->start();
        });
    });
    timer->start();
}

GitIntegration::~GitIntegration() {
    delete d;
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
}

void GitIntegration::unstage(QString file) {
    QProcess* proc = git("reset HEAD " + file);
    proc->waitForFinished();
    proc->deleteLater();
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
}

void GitIntegration::init() {
    if (needsInit()) {
        QProcess* proc = git("init");
        proc->waitForFinished();
        proc->deleteLater();
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
    if (d->gitInstance == "") {
        QStringList instances = findGit();
        if (instances.count() > 0) {
            d->gitInstance = instances.first();
        }
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GIT_TERMINAL_PROMPT", "0");

    QProcess* proc = new QProcess();
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->setWorkingDirectory(d->rootDir);
    proc->setProcessEnvironment(env);
    proc->start(d->gitInstance + " " + args);

    return proc;
}

void GitIntegration::setRootDir(QString rootDir) {
    QMutexLocker locker(&instanceLocker);
    d->rootDir.clear();
    d->rootDir.append(rootDir);
}

QString GitIntegration::rootDir() {
    return d->rootDir;
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
    if (d->knownCommits.contains(hash)) {
        commit = d->knownCommits.value(hash);
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

    if (d->knownCommits.contains(parts.at(0))) {
        commit = d->knownCommits.value(parts.at(0));
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

        populatePointers(commit);

        emit commitInformationAvailable(commit->hash);
    }

    d->knownCommits.insert(commit->hash, commit);

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
    QString oldRootDir = d->rootDir;
    this->setRootDir(rootDir);

    QProcess* proc = git("rev-parse --show-toplevel");
    proc->waitForFinished();

    if (proc->exitCode() == 0) {
        QString rootDir = proc->readAll().trimmed();
        this->setRootDir(rootDir);

        bool retval = false;
        if (oldRootDir != rootDir) {
            retval = true;

            d->knownCommits.clear();
            d->knownBranches.clear();

            //Reset the filesystem watcher
            watcher->removePaths(watcher->files());
            watcher->removePaths(watcher->directories());
            watcherChanged();
        }
        proc->deleteLater();

        return retval;
    } else {
        return true;
    }
}

void GitIntegration::updateWatcher() {
    QDirIterator iterator(d->rootDir + "/.git", QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        if (iterator.fileName() == "." || iterator.fileName() == "..") continue;

        watcher->addPath(iterator.filePath());
    }
}

void GitIntegration::watcherChanged() {
    if (needsInit()) {
        return;
    }
    //Update list of branches
    bool branchesUpdated = false;
    QProcess* branchesProc = git("branch --format=\"%(refname:short)\" -a");
    branchesProc->waitForFinished();

    QStringList branchNames = d->knownBranches.keys();
    while (branchesProc->canReadLine()) {
        QString branchName = branchesProc->readLine().trimmed();
        if (branchName != "" && !d->knownBranches.contains(branchName)) {
            //Cache the branch
            if (!branch(branchName).isNull()) {
                branchesUpdated = true;
            }
        }
        branchNames.removeOne(branchName);
    }

    //Remove all undetected branches
    for (QString name : branchNames) {
        d->knownBranches.remove(name);
        branchesUpdated = true;
    }

    if (branchesUpdated) {
        emit branchesChanged();
    }

    QProcess* branchProc = git("rev-parse --abbrev-ref HEAD");
    branchProc->waitForFinished();

    BranchPointer currentBranch;
    if (branchProc->exitCode() == 0) {
        QString currentBranchName = branchProc->readAll().trimmed();
        currentBranch = branch(currentBranchName);
    }

    if (currentBranch != d->currentBranch) {
        d->currentBranch = currentBranch;
        emit currentBranchChanged();
    }
    branchProc->deleteLater();

    if (!currentBranch.isNull() && !currentBranch->upstream.isNull()) {
        bool didChange = false;

        QProcess* inProc = git("rev-list --count HEAD.." + currentBranch->upstream->name);
        inProc->waitForFinished();
        int pendingIn = inProc->readAll().trimmed().toInt();
        inProc->deleteLater();

        if (d->pendingIn != pendingIn) {
            d->pendingIn = pendingIn;
            didChange = true;
        }

        QProcess* outProc = git("rev-list --count " + currentBranch->upstream->name + "..HEAD");
        outProc->waitForFinished();
        int pendingOut = outProc->readAll().trimmed().toInt();
        outProc->deleteLater();

        if (d->pendingOut != pendingOut) {
            d->pendingOut = pendingOut;
            didChange = true;
        }

        if (didChange) {
            emit pendingCommitsChanged();
        }
    } else {
        d->pendingIn = 0;
        d->pendingOut = 0;
    }

    //Update commits
    d->knownCommits.remove("HEAD");
    CommitPointer head = getCommit("HEAD", false, false);
    if (!head.isNull() && head->hash != d->headCommit) {
        d->headCommit = head->hash;
        emit headCommitChanged();
    }

    updateWatcher();
}

void GitIntegration::checkout(QString item) {
    QProcess* proc = git("checkout " + item);
    proc->waitForFinished();
    proc->deleteLater();
}

GitIntegration::BranchPointer GitIntegration::branch() {
    return d->currentBranch;
}

GitIntegration::BranchPointer GitIntegration::branch(QString name) {
    if (d->knownBranches.contains(name)) {
        return d->knownBranches.value(name);
    }

    if (name.startsWith("(HEAD detached at")) {
        name = "HEAD";
    }

    QProcess* proc = git("branch --list --all --format=\"%(refname:short);%(upstream:short);%(objectname)\" " + name);
    proc->waitForFinished();
    if (proc->exitCode() != 0) return BranchPointer(); //Branch doesn't exist

    QStringList parts = QString(proc->readAll().trimmed()).split(";");
    if (parts.count() < 2) return BranchPointer(); //Invalid branch

    BranchPointer b = BranchPointer(new Branch());
    b->name = parts.at(0);
    if (parts.at(1) != "") {
        b->upstream = branch(parts.at(1));
    }
    b->commit = getCommit(parts.at(2), false, false);

    d->knownBranches.insert(name, b);
    return b;
}

GitIntegration::BranchList GitIntegration::branches() {
    return d->knownBranches.values();
}

void GitIntegration::deleteBranch(BranchPointer branch) {
    if (branch == this->branch()) return; //Don't delete the current branch

    QProcess* proc = git("branch -D " + branch->name);
    proc->waitForFinished();
    proc->deleteLater();
}

QString GitIntegration::myName() {
    QProcess* proc = git("config user.name");
    proc->waitForFinished();

    return proc->readAll().trimmed();
}

int GitIntegration::commitsPendingPush() {
    return d->pendingOut;
}

int GitIntegration::commitsPendingPull() {
    return d->pendingIn;
}

bool GitIntegration::isClean() {
    QProcess* proc = git("status --porcelain");
    proc->waitForFinished();

    if (proc->exitCode() == 0 && proc->readAll().trimmed().isEmpty()) {
        proc->deleteLater();
        return true;
    } else {
        proc->deleteLater();
        return false;
    }
}

void GitIntegration::newBranch(QString name, BranchPointer from) {
    QString gitCommand = "branch " + name;
    if (!from.isNull()) {
        gitCommand.append(" " + from->name);
    }

    QProcess* proc = git(gitCommand);
    proc->waitForFinished();
    proc->deleteLater();
}

void GitIntegration::populatePointers(CommitPointer pointers) {
    pointers->pointer = "";

    if (pointers == getCommit("HEAD", false, false)) {
        //This is the HEAD commit
        pointers->pointer = "HEAD";
        pointers->pointerColor = QColor(0, 150, 0);
    } else {
        for (GitIntegration::BranchPointer branch : branches()) {
            if (branch->commit == pointers) {
                //This is a commit with another branch pointing here
                pointers->pointer = branch->name;
                pointers->pointerColor = QColor(0, 150, 255);
                break;
            }
        }
    }
    pointers->pointersKnown = true;
}

tPromise<void>* GitIntegration::fetch(bool interactive) {
    return new tPromise<void>([=](QString error) {
        if (branch().isNull() || branch()->upstream.isNull()) {
            QProcess* proc = git("fetch");
            proc->waitForFinished();
        }
    });
}

QByteArray GitIntegration::status(bool porcelain) {
    QProcess* proc;
    if (porcelain) {
        proc = git("status --porcelain -z");
    } else {
        proc = git("status");
    }
    proc->waitForFinished();
    return proc->readAll();
}

QByteArray GitIntegration::show(QString item) {
    QProcess* proc = git("show " + item);
    proc->waitForFinished();
    return proc->readAll();
}

void GitIntegration::reset(QString files) {
    QProcess* proc = git("reset " + files);
    proc->waitForFinished();
    proc->deleteLater();
}

void GitIntegration::add(QString file) {
    QProcess* proc = git("add -- " + file);
    proc->waitForFinished();
    proc->deleteLater();
}

void GitIntegration::commit(QString message) {
    QProcess* proc = git("commit -F -");
    proc->write(message.toUtf8());
    proc->closeWriteChannel();
    proc->waitForFinished();
    proc->deleteLater();
}
