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

struct GitIntegrationPrivate;
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

            QString pointer;
            QColor pointerColor;

            bool populated = false;
            bool parentsKnown = false;
            bool pointersKnown = false;
        };
        typedef QSharedPointer<Commit> CommitPointer;
        typedef QList<CommitPointer> CommitList;

        struct Branch {
            QString name;
            QSharedPointer<Branch> upstream;
            CommitPointer commit;
        };
        typedef QSharedPointer<Branch> BranchPointer;
        typedef QList<BranchPointer> BranchList;

        enum ResetType {
            Hard,
            Mixed,
            Soft
        };

        explicit GitIntegration(QString rootDir, QObject *parent = nullptr);
        ~GitIntegration();

        static QStringList findGit();
        static bool isGitEnabled();

    signals:
        void commitsChanged();
        void headCommitChanged();
        void commitInformationAvailable(QString hash);
        void pendingCommitsChanged();

        void branchesChanged();
        void currentBranchChanged();


    public slots:
        CommitPointer getCommit(QString hash, bool populate, bool iterateParents);
        void populatePointers(CommitPointer commit);

        void checkout(QString item, QString args = "");
        BranchPointer branch();
        BranchPointer branch(QString name);
        BranchList branches();
        void newBranch(QString name, BranchPointer from = BranchPointer());
        void deleteBranch(BranchPointer branch);
        int commitsPendingPush();
        int commitsPendingPull();

        bool isClean();

        QString myName();

        tPromise<void>* fetch(bool interactive = false);

        QByteArray status(bool porcelain = true);
        QByteArray show(QString item);

        void reset(QString files = "");
        void resetAll();
        void add(QString file);

        void commit(QString message);
        QString defaultCommitMessage();

        tPromise<void>* merge(QString other);
        void abortMerge();
        bool isConflicting();

        tPromise<void>* pull(QString from = "");
        tPromise<void>* push(QString to = "");
        void setNextCredentials(QString username, QString password);

        void resetTo(QString commit, ResetType type);

        void init();
        bool needsInit();

        tPromise<CommitList>* commits(QString branch = "HEAD");

        bool setNewRootDir(QString rootDir);
        void setRootDir(QString rootDir);
        QString rootDir();

    private slots:
        void watcherChanged();

    private:
        GitIntegrationPrivate* d;

        QProcess* git(QString args);
        void clearCredentials();
        QMutex instanceLocker;

        QFileSystemWatcher* watcher;
        void updateWatcher();
};
Q_DECLARE_METATYPE(GitIntegration::CommitPointer)
Q_DECLARE_METATYPE(GitIntegration::BranchPointer)

#endif // GITINTEGRATION_H
