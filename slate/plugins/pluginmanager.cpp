#include "pluginmanager.h"
#include <QMessageBox>

#ifdef Q_OS_MAC
    extern QString bundlePath;
#endif

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{
    //Load all plugins
    QStringList pluginSearchPaths;
    #if defined(Q_OS_WIN)
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../../FileBackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/filebackends/");
    #elif defined(Q_OS_MAC)
        pluginSearchPaths.append(bundlePath + "/Contents/filebackends/");
    #elif (defined Q_OS_UNIX)
        pluginSearchPaths.append(QApplication::applicationDirPath() + "/../FileBackends/");
        pluginSearchPaths.append("/usr/share/theslate/filebackends/");
        pluginSearchPaths.append(QApplication::applicationDirPath() + "../../share/theslate/filebackends/");
    #endif

    QObjectList availablePlugins;
    availablePlugins.append(QPluginLoader::staticInstances());

    for (QString path : pluginSearchPaths) {
        QDirIterator it(path, QDirIterator::Subdirectories);
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
