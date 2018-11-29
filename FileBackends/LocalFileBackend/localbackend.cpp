#include "localbackend.h"

LocalBackend::LocalBackend(QString filename, QObject *parent) : FileBackend(parent)
{
    this->fileName = filename;
    watcher = new QFileSystemWatcher(QStringList() << filename);
    connect(watcher, &QFileSystemWatcher::fileChanged, [=] {
        emit remoteFileEdited();
    });
}

tPromise<void>* LocalBackend::save(QByteArray fileContents) {
    return new tPromise<void>([=](QString &error) {
        QSignalBlocker blocker(watcher);

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            error = "Can't open file";
        } else {
            file.write(fileContents);
            file.close();

            QThread::msleep(500);
        }
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

