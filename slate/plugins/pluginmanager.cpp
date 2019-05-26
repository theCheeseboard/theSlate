#include "pluginmanager.h"
#include <QMessageBox>
#include <tapplication.h>

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{
    //Load all plugins
    QStringList pluginSearchPaths;
    #if defined(Q_OS_WIN)
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../../FileBackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/filebackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../../AuxiliaryPanes/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/auxiliarypanes/");
    #elif defined(Q_OS_MAC)
        pluginSearchPaths.append(tApplication::macOSBundlePath() + "/Contents/filebackends/");
        pluginSearchPaths.append(tApplication::macOSBundlePath() + "/Contents/auxiliarypanes/");
    #elif (defined Q_OS_UNIX)
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../FileBackends/");
        pluginSearchPaths.append("/usr/share/theslate/filebackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../share/theslate/filebackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../AuxiliaryPanes/");
        pluginSearchPaths.append("/usr/share/theslate/auxiliarypanes/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../share/theslate/auxiliarypanes/");
    #endif

    QObjectList availablePlugins;
    availablePlugins.append(QPluginLoader::staticInstances());

    for (QString path : pluginSearchPaths) {
        QDirIterator it(QDir::cleanPath(path), QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QPluginLoader loader(it.filePath());
            QObject* plugin = loader.instance();
            if (plugin) {
                availablePlugins.append(plugin);
            }
        }
    }

    QStringList errors;
    for (QObject* obj : availablePlugins) {
        FileBackendPlugin* file = qobject_cast<FileBackendPlugin*>(obj);
        if (file) {
            QList<FileBackendFactory*> backends = file->getFactories();

            //Find the local file backend
            if (localFileBackend == nullptr) {
                for (FileBackendFactory* factory : backends) {
                    if (factory->name() == "Local File") localFileBackend = factory;
                }
            }

            fileFactories.append(backends);
        }

        AuxiliaryPanePlugin* aux = qobject_cast<AuxiliaryPanePlugin*>(obj);
        if (aux) {
            auxiliaryPanePlugins.append(aux);
        }
    }

    if (localFileBackend == nullptr) {
        QMessageBox::critical(nullptr, tr("theSlate may not work properly"), tr("The Local File Backend was unable to be loaded. theSlate may quit unexpectedly."), QMessageBox::Ok, QMessageBox::Ok);
    }
}

QList<FileBackendFactory*> PluginManager::fileBackends() {
    return fileFactories;
}

FileBackendFactory* PluginManager::getLocalFileBackend() {
    return localFileBackend;
}

QList<AuxiliaryPaneCapabilities> PluginManager::auxiliaryPanes() {
    QList<AuxiliaryPaneCapabilities> availablePanes;
    for (AuxiliaryPanePlugin* plugin : auxiliaryPanePlugins) {
        availablePanes.append(plugin->getPanes());
    }
    return availablePanes;
}

QList<AuxiliaryPaneCapabilities> PluginManager::auxiliaryPanesForUrl(QUrl url) {
    QList<AuxiliaryPaneCapabilities> availablePanes;
    for (AuxiliaryPanePlugin* plugin : auxiliaryPanePlugins) {
        availablePanes.append(plugin->getPanesForUrl(url));
    }
    return availablePanes;
}
