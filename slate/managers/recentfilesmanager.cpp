#include "recentfilesmanager.h"

#include <QUrl>
#include <QFile>

RecentFilesManager::RecentFilesManager(QObject *parent) : QObject(parent)
{
    files = settings.value("state/recents", QStringList()).toStringList();

    for (auto i = files.begin(); i != files.end(); i++) {
        if (!QFile(QUrl(*i).toLocalFile()).exists()) {
            files.erase(i);
        }
    }
}

QStringList RecentFilesManager::getFiles() {
    return files;
}

void RecentFilesManager::putFile(QString file) {
    if (files.contains(file)) files.removeAll(file);
    files.prepend(file);
    if (files.length() > 10) files.removeLast();
    settings.setValue("state/recents", files);

    emit filesUpdated();
}

void RecentFilesManager::clear() {
    files.clear();
    settings.setValue("state/recents", files);

    emit filesUpdated();
}
