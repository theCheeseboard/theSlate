#ifndef LOCALBACKEND_H
#define LOCALBACKEND_H

#include <QObject>
#include "../../slate/plugins/filebackend.h"

class LocalBackend : public FileBackend
{
        Q_OBJECT
    public:
        explicit LocalBackend(QString filename, QObject *parent = nullptr);

    signals:

    public slots:
        tPromise<bool>* save(QByteArray fileContents);
        tPromise<QByteArray>* load();

        QString documentTitle();
        QUrl url();
        FileBackend* openFromUrl(QUrl url);

    private:
        QString fileName;
};

#endif // LOCALBACKEND_H
