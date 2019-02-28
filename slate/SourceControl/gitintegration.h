#ifndef GITINTEGRATION_H
#define GITINTEGRATION_H

#include <QObject>
#include <QDir>
#include <QProcess>
#include <QMutex>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <tpromise.h>

class GitTask : public QObject
{
    Q_OBJECT

    public:
        explicit GitTask(QObject* parent = nullptr);

    signals:
        void finished(QString message = "");
        void failed(QString message);
        void message(QString message);
        void progress(int progress, int maximum);
        void output(QString line);

    public slots:
        void appendToBuffer(QByteArray append);
        QByteArray buffer();

    private:
        QByteArray buf;
};

class GitIntegration : public QObject
{
    Q_OBJECT
    public:
        struct Commit {
            QString hash;
            QString author, authorEmail;
            QString committer, committerEmail;
            QDateTime commitDate;

            QList<QSharedPointer<Commit>> parents;

            QString message;

            bool populated = false;
            bool parentsKnown = false;
        };
        typedef QSharedPointer<Commit> CommitPointer;
        typedef QList<CommitPointer> CommitList;

        explicit GitIntegration(QString rootDir, QObject *parent = nullptr);

        static QStringList findGit();

    signals:
        void reloadStatusNeeded();
        void commitsChanged();
        void commitInformationAvailable(QString hash);

        void headCommitChanged();
        void currentBranchChanged();

    public slots:
        tPromise<QStringList>* reloadStatus();

        CommitPointer getCommit(QString hash, bool populate, bool iterateParents);

        void checkout(QString item);
        QString branch();
        QString upstreamBranch();
        int commitsPendingPush();
        int commitsPendingPull();

        QString myName();

        void add(QString file);
        void rm(QString file, bool cache = false);
        void unstage(QString file);
        void abortMerge();
        QString commit(QString message);

        GitTask* pull();
        GitTask* push();

        void init();
        bool needsInit();

        tPromise<CommitList>* commits(QString branch = "HEAD");

        bool setNewRootDir(QString rootDir);
        void setRootDir(QString rootDir);

    private slots:
        void watcherChanged();

    private:
        QString rootDir;
        QString gitInstance;

        QString headCommit;
        QString currentBranch;
        QString upstream;
        int pendingIn = 0;
        int pendingOut = 0;

        QProcess* git(QString args);
        QMutex instanceLocker;

        QFileSystemWatcher* watcher;
        void updateWatcher();

        QMap<QString, CommitPointer> knownCommits;
};
Q_DECLARE_METATYPE(GitIntegration::CommitPointer)

#endif // GITINTEGRATION_H
