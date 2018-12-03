#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QObject>
#include <QSettings>

class RecentFilesManager : public QObject
{
        Q_OBJECT
    public:
        explicit RecentFilesManager(QObject *parent = nullptr);

        QStringList getFiles();
    signals:
        void filesUpdated();

    public slots:
        void putFile(QString file);
        void clear();

    private:
        QSettings settings;

        QStringList files;
};

#endif // RECENTFILESMANAGER_H
