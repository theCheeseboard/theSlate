#include "localbackend.h"

LocalBackend::LocalBackend(QString filename, QObject *parent) : FileBackend(parent)
{
    this->fileName = filename;
}

tPromise<bool>* LocalBackend::save(QByteArray fileContents) {
    return new tPromise<bool>([=](QString &error) {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            error = "Can't open file";
            return false;
        } else {
            file.write(fileContents);
            file.close();
            return true;
        }
    });
}

tPromise<QByteArray>* LocalBackend::load() {
    return new tPromise<QByteArray>([=](QString& error) {
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

