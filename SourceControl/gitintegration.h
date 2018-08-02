#ifndef GITINTEGRATION_H
#define GITINTEGRATION_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>
#include <QProcess>

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
        explicit GitIntegration(QDir rootDir, QObject *parent = nullptr);

        static QStringList findGit();

    signals:
        void reloadStatusNeeded();

    public slots:
        QStringList reloadStatus();

        void add(QString file);
        void rm(QString file, bool cache = false);
        void unstage(QString file);
        void abortMerge();

        GitTask* pull();

        void init();
        bool needsInit();

    private:
        QDir rootDir;
        QString gitInstance;

        QProcess* git(QString args);
        QFileSystemWatcher* watcher;
};

#endif // GITINTEGRATION_H
