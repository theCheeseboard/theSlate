#ifndef HTTPBACKEND_H
#define HTTPBACKEND_H

#include "../../slate/plugins/filebackend.h"
#include <QNetworkAccessManager>

class HttpBackend : public FileBackend
{
    Q_OBJECT

    public:
        explicit HttpBackend(QUrl url, QObject *parent = nullptr);

    signals:

    public slots:
        tPromise<void>* save(QByteArray fileContents);
        tPromise<QByteArray>* load();

        QString documentTitle();
        QUrl url();
        bool readOnly();

        void setRedirect(bool redirect);

    private:
        QUrl fileUrl;
        bool redirect = false;

        QString currentAuthRealm, currentAuthHost, currentAuthUsername, currentAuthPassword;
};

#endif // HTTPBACKEND_H
