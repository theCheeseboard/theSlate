#ifndef CODEMODELOBJECT_H
#define CODEMODELOBJECT_H

#include <QDir>
#include <QObject>

#include "directoryobject.h"
#include "targetobject.h"

struct CodemodelObjectPrivate;
class CodemodelObject : public QObject {
        Q_OBJECT
    public:
        explicit CodemodelObject(QDir replyDir, QString codemodelFile, QObject* parent = nullptr);
        ~CodemodelObject();

        QStringList targets();
        TargetObjectPtr target(QString target);

        DirectoryObjectPtr directory(int index);
        DirectoryObjectPtr directoryForTarget(QString target);

    signals:

    private:
        CodemodelObjectPrivate* d;
};

typedef QScopedPointer<CodemodelObject> CodemodelObjectPtr;

#endif // CODEMODELOBJECT_H
