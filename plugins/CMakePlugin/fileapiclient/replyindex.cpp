#include "replyindex.h"

#include <QJsonDocument>
#include <QJsonObject>

struct ReplyIndexPrivate {
        QDir replyDir;
        QJsonObject indexFile;
};

ReplyIndex::ReplyIndex(QDir buildDir, QObject* parent) :
    QObject{parent} {
    d = new ReplyIndexPrivate;
    d->replyDir = buildDir.absoluteFilePath(".cmake/api/v1/reply");

    // Find the index
    auto indexFiles = d->replyDir.entryList({"index-*.json"}, QDir::NoFilter, QDir::Name);
    if (!indexFiles.isEmpty()) {
        auto indexFile = QFile(d->replyDir.absoluteFilePath(indexFiles.last()));
        indexFile.open(QFile::ReadOnly);
        d->indexFile = QJsonDocument::fromJson(indexFile.readAll()).object();
        indexFile.close();
    }
}

ReplyIndex::~ReplyIndex() {
    delete d;
}

CodemodelObjectPtr ReplyIndex::codemodel() {
    return CodemodelObjectPtr(new CodemodelObject(d->replyDir, d->indexFile.value("reply").toObject().value("codemodel-v2").toObject().value("jsonFile").toString()));
}
