#ifndef GITINTEGRATION_H
#define GITINTEGRATION_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>
#include <QProcess>

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

        void init();
        bool needsInit();

    private:
        QDir rootDir;
        QString gitInstance;

        QProcess* git(QString args);
};

#endif // GITINTEGRATION_H
