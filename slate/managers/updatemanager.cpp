#include "updatemanager.h"

#include <QApplication>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>

UpdateManager::UpdateManager(QObject *parent) : QObject(parent)
{
    QApplication::setApplicationVersion(versionString());
    checkForUpdates();
}

QString UpdateManager::versionString() {
    return versionString(version);
}

QString UpdateManager::versionString(const int params[]) {
    return QString::number(params[0]) + "." + QString::number(params[1]) + (params[2] == 0 ? "" : ("." + QString::number(params[2])));
}

QAction* UpdateManager::getCheckForUpdatesAction() {
    QAction* a = new QAction;
    updateAction(a);
    a->setMenuRole(QAction::ApplicationSpecificRole);
    connect(a, &QAction::triggered, this, &UpdateManager::checkForUpdates);
    connect(a, &QAction::destroyed, [=] {
        actions.removeAll(a);
    });

    actions.append(a);

    return a;
}

void UpdateManager::updateAllActions() {
    for (QAction* action : actions) {
        updateAction(action);
    }
}

void UpdateManager::updateAction(QAction* action) {
    switch (s) {
        case Checking:
            action->setText(tr("Checking for updates..."));
            action->setEnabled(false);
            break;
        case Idle:
            action->setText(tr("Check for updates"));
            action->setEnabled(false);
            break;
        case NewUpdateAvailable:
            action->setText(tr("Update to %1 available").arg(versionString(newUpdateVersion)));
            action->setEnabled(true);
            break;
    }
}

void UpdateManager::checkForUpdates() {
    //Check for updates only on Windows or macOS
    #if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        if (s == NewUpdateAvailable) {
            if (QFile(QApplication::applicationDirPath() + "/uninstall.exe").exists()) {
                //Use theInstaller to update theSlate
                QProcess::startDetached(QApplication::applicationDirPath() + "/uninstall.exe", {"--update-from-app"});
                return;
            }

            //Open the website to download the latest version
            QDesktopServices::openUrl(QUrl("https://vicr123.com/theslate/download.html?update=true"));
        } else {
            s = Checking;

            QNetworkRequest req(QUrl("https://vicr123.com/theslate/theinstaller/installer.json"));
            req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            QNetworkReply* reply = mgr.get(req);
            connect(reply, &QNetworkReply::finished, [=] {
                s = Idle;
                updateAllActions();

                if (reply->error() != QNetworkReply::NoError) return;

                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                if (!doc.isObject()) return;

                QJsonObject obj = doc.object();
                if (!obj.contains("version")) return;

                QJsonObject versions = obj.value("version").toObject();

                int newVersion[] = {
                    versions.value("major").toInt(),
                    versions.value("minor").toInt(),
                    versions.value("bugfix").toInt()
                };

                bool isNewVersion = false;
                for (int i = 0; i < 3; i++) {
                    if (version[i] < newVersion[i]) {
                        isNewVersion = true;
                    }
                }

                if (isNewVersion) {
                    for (int i = 0; i < 3; i++) {
                        newUpdateVersion[i] = newVersion[i];
                    }
                    s = NewUpdateAvailable;
                    emit updateAvailable();
                }

                updateAllActions();
            });

            updateAllActions();
        }
    #else
        s = Idle;
        updateAllActions();
    #endif
}
