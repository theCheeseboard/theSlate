#ifndef REPLYINDEX_H
#define REPLYINDEX_H

#include <QDir>
#include <QObject>
#include <QScopedPointer>

#include "codemodelobject.h"

struct ReplyIndexPrivate;
class ReplyIndex : public QObject {
        Q_OBJECT
    public:
        explicit ReplyIndex(QDir buildDir, QObject* parent = nullptr);
        ~ReplyIndex();

        CodemodelObjectPtr codemodel();

    signals:

    private:
        ReplyIndexPrivate* d;
};

typedef QScopedPointer<ReplyIndex> ReplyIndexPtr;

#endif // REPLYINDEX_H
