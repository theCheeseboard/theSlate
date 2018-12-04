#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QAction>
#include <QNetworkAccessManager>

class UpdateManager : public QObject
{
        Q_OBJECT
    public:
        explicit UpdateManager(QObject *parent = nullptr);

        enum State {
            Idle,
            Checking,
            NewUpdateAvailable
        };

        QString versionString();
        QAction* getCheckForUpdatesAction();

    signals:
        void updateAvailable();

    public slots:
        void checkForUpdates();

    private:
        const int version[3] = {
            0, 4, 1
        };

        QList<QAction*> actions;
        void updateAction(QAction* action);
        void updateAllActions();

        QString versionString(const int params[]);

        State s = Idle;
        int newUpdateVersion[3];

        QNetworkAccessManager mgr;
};

#endif // UPDATEMANAGER_H
