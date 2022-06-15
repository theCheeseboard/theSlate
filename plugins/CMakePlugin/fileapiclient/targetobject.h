#ifndef TARGETOBJECT_H
#define TARGETOBJECT_H

#include <QDir>
#include <QObject>

struct TargetObjectPrivate;
class TargetObject : public QObject {
        Q_OBJECT
    public:
        explicit TargetObject(QDir replyDir, QString targetFile, QObject* parent = nullptr);
        ~TargetObject();

        enum TargetType {
            Executable,
            StaticLibrary,
            SharedLibrary,
            ModuleLibrary,
            ObjectLibrary,
            InterfaceLibrary,
            Utility,
            UnknownType
        };

        QStringList artifacts();
        TargetType type();

    signals:

    private:
        TargetObjectPrivate* d;
};

typedef QScopedPointer<TargetObject> TargetObjectPtr;

#endif // TARGETOBJECT_H
