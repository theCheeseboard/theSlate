#include "localbackend.h"
#include <QStorageInfo>

LocalBackend::LocalBackend(QString filename, QObject *parent) : FileBackend(parent)
{
    this->fileName = filename;
    watcher = new QFileSystemWatcher(QStringList() << filename);
    connect(watcher, &QFileSystemWatcher::fileChanged, [=] {
        if (QFile::exists(filename)) {
            emit remoteFileEdited();
        } else {
            emit remoteFileRemoved();
        }
    });
}

tPromise<void>* LocalBackend::save(QByteArray fileContents) {
    return new tPromise<void>([=](QString &error) {
        QSignalBlocker blocker(watcher);

        QStorageInfo storage(QFileInfo(fileName).dir());
        qDebug() << storage.isValid();
        if (storage.isValid() && storage.bytesAvailable() < fileContents.length()) {
            error = "Disk Full";
            return;
        }

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            if (!(file.permissions() & QFile::WriteUser)) {
                error = "Permissions";
            } else {
                error = "Can't open file";
            }
            return;
        }

        qint64 written = file.write(fileContents);
        if (written == -1) {
            error = "Disk Full";
            return;
        }

        file.close();
        QThread::msleep(500);

        watcher->removePaths(watcher->files());
        watcher->addPaths(QStringList() << fileName);
    });
}

tPromise<QByteArray>* LocalBackend::load() {
    return new tPromise<QByteArray>([=](QString& error) {
        QSignalBlocker blocker(watcher);
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            error = "Can't open file";
            return QByteArray();
        } else {
            return file.readAll();
        }
    });
}

QString LocalBackend::documentTitle() {
    QFileInfo info(fileName);
    return info.fileName();
}

QUrl LocalBackend::url() {
    return QUrl::fromLocalFile(fileName);
}

bool LocalBackend::readOnly() {
    return false;
}
