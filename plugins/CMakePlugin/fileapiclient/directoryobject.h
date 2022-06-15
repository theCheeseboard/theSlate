#ifndef DIRECTORYOBJECT_H
#define DIRECTORYOBJECT_H

#include <QDir>
#include <QObject>

struct DirectoryObjectPrivate;
class DirectoryObject : public QObject {
        Q_OBJECT
    public:
        explicit DirectoryObject(QDir replyDir, QString directoryFile, QObject* parent = nullptr);
        ~DirectoryObject();

        QString buildPath();

    signals:

    private:
        DirectoryObjectPrivate* d;
};

typedef QScopedPointer<DirectoryObject> DirectoryObjectPtr;

#endif // DIRECTORYOBJECT_H
