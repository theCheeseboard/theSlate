#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include "filebackend.h"
#include "syntaxhighlighting.h"

class PluginManager : public QObject
{
        Q_OBJECT
    public:
        explicit PluginManager(QObject *parent = nullptr);

        QList<FileBackendFactory*> fileBackends();
        QList<SyntaxHighlighting*> syntaxHighlighters();
        FileBackendFactory* getLocalFileBackend();
    signals:

    public slots:

    private:
        QList<SyntaxHighlighting*> highlightingDefinitions;
        QList<FileBackendFactory*> fileFactories;

        FileBackendFactory* localFileBackend = nullptr;
};

#endif // PLUGINMANAGER_H
