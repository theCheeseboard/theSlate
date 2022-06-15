#include "targetobject.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

struct TargetObjectPrivate {
        QDir replyDir;
        QJsonObject targetFile;
};

TargetObject::TargetObject(QDir replyDir, QString targetFile, QObject* parent) :
    QObject{parent} {
    d = new TargetObjectPrivate();
    d->replyDir = replyDir;

    QFile targetJsonFile(replyDir.absoluteFilePath(targetFile));
    targetJsonFile.open(QFile::ReadOnly);
    d->targetFile = QJsonDocument::fromJson(targetJsonFile.readAll()).object();
    targetJsonFile.close();
}

TargetObject::~TargetObject() {
    delete d;
}

QStringList TargetObject::artifacts() {
    QStringList artifacts;
    auto artifactsJson = d->targetFile.value("artifacts").toArray();
    for (auto artifact : artifactsJson) {
        artifacts.append(artifact.toObject().value("path").toString());
    }
    return artifacts;
}

TargetObject::TargetType TargetObject::type() {
    auto type = d->targetFile.value("type").toString();
    if (type == "EXECUTABLE") {
        return Executable;
    } else if (type == "STATIC_LIBRARY") {
        return StaticLibrary;
    } else if (type == "SHARED_LIBRARY") {
        return SharedLibrary;
    } else if (type == "MODULE_LIBRARY") {
        return ModuleLibrary;
    } else if (type == "OBJECT_LIBRARY") {
        return ObjectLibrary;
    } else if (type == "INTERFACE_LIBRARY") {
        return InterfaceLibrary;
    } else if (type == "UTILITY") {
        return Utility;
    }
    return UnknownType;
}
