#ifndef LOCALBACKEND_H
#define LOCALBACKEND_H

#include <QObject>
#include <QFileSystemWatcher>
#include "../../slate/plugins/filebackend.h"

class LocalBackend : public FileBackend
{
        Q_OBJECT
    public:
        explicit LocalBackend(QString filename, QObject *parent = nullptr);

    signals:

    public slots:
        tPromise<void>* save(QByteArray fileContents);
        tPromise<QByteArray>* load();

        QString documentTitle();
        QUrl url();
        bool readOnly();

    private:
        QString fileName;
        QFileSystemWatcher* watcher;
};

#endif // LOCALBACKEND_H
