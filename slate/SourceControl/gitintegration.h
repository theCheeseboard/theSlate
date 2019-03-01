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

        explicit GitIntegration(QString rootDir, QObject *parent = nullptr);
        ~GitIntegration();

        static QStringList findGit();

    signals:
        void commitsChanged();
        void headCommitChanged();
        void commitInformationAvailable(QString hash);
        void pendingCommitsChanged();

        void branchesChanged();
        void currentBranchChanged();


    public slots:
        tPromise<QStringList>* reloadStatus();

        CommitPointer getCommit(QString hash, bool populate, bool iterateParents);
        void populatePointers(CommitPointer commit);

        void checkout(QString item);
        BranchPointer branch();
        BranchPointer branch(QString name);
        BranchList branches();
        void newBranch(QString name, BranchPointer from = BranchPointer());
        void deleteBranch(BranchPointer branch);
        int commitsPendingPush();
        int commitsPendingPull();

        bool isClean();

        QString myName();

        tPromise<void>* fetch();

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
        GitIntegrationPrivate* d;

        QProcess* git(QString args);
        QMutex instanceLocker;

        QFileSystemWatcher* watcher;
        void updateWatcher();
};
Q_DECLARE_METATYPE(GitIntegration::CommitPointer)
Q_DECLARE_METATYPE(GitIntegration::BranchPointer)

#endif // GITINTEGRATION_H
