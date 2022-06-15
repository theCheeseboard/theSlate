#include "directoryobject.h"

#include <QJsonDocument>
#include <QJsonObject>

struct DirectoryObjectPrivate {
        QDir replyDir;
        QJsonObject directoryFile;
};

DirectoryObject::DirectoryObject(QDir replyDir, QString directoryFile, QObject* parent) :
    QObject{parent} {
    d = new DirectoryObjectPrivate();
    d->replyDir = replyDir;

    QFile directoryJsonFile(replyDir.absoluteFilePath(directoryFile));
    directoryJsonFile.open(QFile::ReadOnly);
    d->directoryFile = QJsonDocument::fromJson(directoryJsonFile.readAll()).object();
    directoryJsonFile.close();
}

DirectoryObject::~DirectoryObject() {
    delete d;
}

QString DirectoryObject::buildPath() {
    return d->directoryFile.value("paths").toObject().value("build").toString();
}
