#include "plugin.h"

#include <QAction>
#include <QFileDialog>
#include <QEventLoop>
#include "localbackend.h"

Plugin::Plugin() : FileBackendPlugin()
{

}

QList<FileBackendFactory*> Plugin::getFactories() {
    QList<FileBackendFactory*> factories;
    factories.append(new LocalBackendFactory);
    return factories;
}

QAction* LocalBackendFactory::makeOpenAction(QWidget* parent, std::function<QVariant(QString)> getOption) {
    QAction* a = new QAction();
    a->setText(tr("Open"));
    a->setIcon(QIcon::fromTheme("document-open"));
    connect(a, &QAction::triggered, [=] {
        QEventLoop* loop = new QEventLoop();
        QFileDialog* openDialog = new QFileDialog(parent, Qt::Sheet);
        openDialog->setWindowModality(Qt::WindowModal);
        openDialog->setAcceptMode(QFileDialog::AcceptOpen);
        openDialog->setFileMode(QFileDialog::ExistingFile);

        QVariant currentDirectory = getOption("currentDirectory");
        openDialog->setDirectory(currentDirectory.isNull() ? QDir::home() : QDir(currentDirectory.toString()));

        openDialog->setNameFilter("All Files (*)");
        connect(openDialog, SIGNAL(finished(int)), openDialog, SLOT(deleteLater()));
        connect(openDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        openDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (openDialog->result() == QDialog::Accepted) {
            //Create new file backend
            emit openFile(new LocalBackend(openDialog->selectedFiles().first()));
        }
    });
    return a;
}

QString LocalBackendFactory::name() {
    return "Local File";
}

FileBackend* LocalBackendFactory::openFromUrl(QUrl url) {
    if (!url.isLocalFile()) {
        return nullptr;
    } else {
        return new LocalBackend(url.toLocalFile());
    }
}

QUrl LocalBackendFactory::askForUrl(QWidget* parent, std::function<QVariant(QString)> getOption, bool* ok) {
    QEventLoop* loop = new QEventLoop();
    QFileDialog* saveDialog = new QFileDialog(parent->window(), Qt::Sheet);
    saveDialog->setWindowModality(Qt::WindowModal);
    saveDialog->setAcceptMode(QFileDialog::AcceptSave);
    saveDialog->setNameFilters(QStringList() << "Text File (*.txt)"
                                             << "All Files (*)");

    QVariant currentDirectory = getOption("currentDirectory");
    saveDialog->setDirectory(currentDirectory.isNull() ? QDir::home() : QDir(currentDirectory.toString()));

    connect(saveDialog, SIGNAL(finished(int)), saveDialog, SLOT(deleteLater()));
    connect(saveDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
    saveDialog->show();

    //Block until dialog is finished
    loop->exec();
    loop->deleteLater();

    if (saveDialog->result() == QDialog::Accepted) {
        if (ok != nullptr) *ok = true;
        return QUrl::fromLocalFile(saveDialog->selectedFiles().first());
    } else {
        if (ok != nullptr) *ok = false;
        return QUrl();
    }
}
