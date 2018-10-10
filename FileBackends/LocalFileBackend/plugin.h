#ifndef PLUGIN_H
#define PLUGIN_H

#include "../../slate/plugins/filebackend.h"

class LocalBackendFactory : public FileBackendFactory
{
    Q_OBJECT

    public:
        LocalBackendFactory(QObject* parent = nullptr) : FileBackendFactory(parent) {}

        QAction* makeOpenAction(QWidget* parent);
};

class Plugin : public QObject, public FileBackendPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID THESLATE_FILEBACKEND_IID FILE "LocalFileBackend.json")
    Q_INTERFACES(FileBackendPlugin)

    public:
        Plugin();

        QList<FileBackendFactory*> getFactories();

};

#endif // PLUGIN_H
