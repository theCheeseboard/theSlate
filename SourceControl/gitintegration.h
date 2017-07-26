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
    QProcess* proc;
};

#endif // GITINTEGRATION_H
