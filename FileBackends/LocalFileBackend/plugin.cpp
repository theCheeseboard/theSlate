#include "plugin.h"

#include <QAction>
#include <QFileDialog>
#include <QEventLoop>

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
    a->setText(tr("Open"));
    a->setIcon(QIcon::fromTheme("document-open"));
    connect(a, &QAction::triggered, [=] {
        QEventLoop* loop = new QEventLoop();
        QFileDialog* openDialog = new QFileDialog(parent, Qt::Sheet);
        openDialog->setWindowModality(Qt::WindowModal);
        openDialog->setAcceptMode(QFileDialog::AcceptOpen);

        openDialog->setDirectory(QDir::home());

        openDialog->setNameFilter("All Files (*)");
        connect(openDialog, SIGNAL(finished(int)), openDialog, SLOT(deleteLater()));
        connect(openDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        openDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (openDialog->result() == QDialog::Accepted) {
            //Create new file backend
            //newTab(openDialog->selectedFiles().first());
            emit openFile(nullptr);
        }
    });
    return a;
}
