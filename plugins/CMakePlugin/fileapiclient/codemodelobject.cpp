#include "codemodelobject.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

struct CodemodelObjectPrivate {
        QDir replyDir;
        QJsonObject codemodelFile;
};

CodemodelObject::CodemodelObject(QDir replyDir, QString codemodelFile, QObject* parent) :
    QObject{parent} {
    d = new CodemodelObjectPrivate();
    d->replyDir = replyDir;

    QFile codemodelJsonFile(replyDir.absoluteFilePath(codemodelFile));
    codemodelJsonFile.open(QFile::ReadOnly);
    d->codemodelFile = QJsonDocument::fromJson(codemodelJsonFile.readAll()).object();
    codemodelJsonFile.close();
}

CodemodelObject::~CodemodelObject() {
    delete d;
}

QStringList CodemodelObject::targets() {
    QStringList targets;
    auto targetsJson = d->codemodelFile.value("configurations").toArray().at(0).toObject().value("targets").toArray();
    for (auto value : targetsJson) {
        targets.append(value.toObject().value("name").toString());
    }
    return targets;
}

TargetObjectPtr CodemodelObject::target(QString target) {
    int index = this->targets().indexOf(target);
    if (index == -1) return TargetObjectPtr();

    auto targetsJson = d->codemodelFile.value("configurations").toArray().at(0).toObject().value("targets").toArray();
    auto targetJson = targetsJson.at(index).toObject();

    return TargetObjectPtr(new TargetObject(d->replyDir, targetJson.value("jsonFile").toString()));
}

DirectoryObjectPtr CodemodelObject::directory(int index) {
    auto directoriesJson = d->codemodelFile.value("configurations").toArray().at(0).toObject().value("directories").toArray();
    auto directoryJson = directoriesJson.at(index).toObject();

    return DirectoryObjectPtr(new DirectoryObject(d->replyDir, directoryJson.value("jsonFile").toString()));
}

DirectoryObjectPtr CodemodelObject::directoryForTarget(QString target) {
    int index = this->targets().indexOf(target);
    if (index == -1) return DirectoryObjectPtr();

    auto targetsJson = d->codemodelFile.value("configurations").toArray().at(0).toObject().value("targets").toArray();
    auto targetJson = targetsJson.at(index).toObject();

    return directory(targetJson.value("directoryIndex").toInt());
}
