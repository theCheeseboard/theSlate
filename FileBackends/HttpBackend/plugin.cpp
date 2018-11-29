#include "plugin.h"

#include <QAction>
#include <QFileDialog>
#include <QEventLoop>
#include "opendialog.h"
#include "httpbackend.h"

Plugin::Plugin() : FileBackendPlugin()
{

}

QList<FileBackendFactory*> Plugin::getFactories() {
    QList<FileBackendFactory*> factories;
    factories.append(new LocalBackendFactory);
    return factories;
}

QAction* LocalBackendFactory::makeOpenAction(QWidget* parent) {
    QAction* a = new QAction();
    a->setText(tr("HTTP URL"));
    a->setIcon(QIcon::fromTheme("network-wireless"));
    connect(a, &QAction::triggered, [=] {
        QEventLoop* loop = new QEventLoop();
        OpenDialog* openDialog = new OpenDialog(parent);
        openDialog->setWindowFlag(Qt::Sheet);
        openDialog->setWindowModality(Qt::WindowModal);

        connect(openDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        openDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (openDialog->result() == QDialog::Accepted) {
            //Create new file backend
            HttpBackend* backend = new HttpBackend(openDialog->currentUrl());
            backend->setRedirect(openDialog->redirect());
            emit openFile(backend);
        }

        openDialog->deleteLater();
    });
    return a;
}

QString LocalBackendFactory::name() {
    return "HTTP";
}

FileBackend* LocalBackendFactory::openFromUrl(QUrl url) {
    if (!url.isLocalFile()) {
        return nullptr;
    } else {
        return new HttpBackend(url);
    }
}

QUrl LocalBackendFactory::askForUrl(QWidget* parent, bool* ok) {
    //Don't allow saving for now
    *ok = false;
    return QUrl();
}
