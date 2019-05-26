#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include "filebackend.h"
#include "auxiliarypane.h"

class PluginManager : public QObject
{
        Q_OBJECT
    public:
        explicit PluginManager(QObject *parent = nullptr);

        QList<FileBackendFactory*> fileBackends();
        FileBackendFactory* getLocalFileBackend();

        QList<AuxiliaryPaneCapabilities> auxiliaryPanes();
        QList<AuxiliaryPaneCapabilities> auxiliaryPanesForUrl(QUrl url);
        AuxiliaryPane* newAuxiliaryPane(QString name);
    signals:

    public slots:

    private:
        QList<FileBackendFactory*> fileFactories;
        QList<AuxiliaryPanePlugin*> auxiliaryPanePlugins;

        FileBackendFactory* localFileBackend = nullptr;
};

#endif // PLUGINMANAGER_H
