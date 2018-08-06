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
        explicit GitIntegration(QString rootDir, QObject *parent = nullptr);

        static QStringList findGit();

    signals:
        void reloadStatusNeeded();

    public slots:
        tPromise<QStringList>* reloadStatus();

        void add(QString file);
        void rm(QString file, bool cache = false);
        void unstage(QString file);
        void abortMerge();
        QString commit(QString message);

        GitTask* pull();
        GitTask* push();

        void init();
        bool needsInit();

        void setRootDir(QString rootDir);

    private:
        QString rootDir;
        QString gitInstance;

        QProcess* git(QString args);
        QMutex instanceLocker;
};

#endif // GITINTEGRATION_H
