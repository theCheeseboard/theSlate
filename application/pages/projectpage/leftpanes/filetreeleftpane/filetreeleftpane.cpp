#include "filetreeleftpane.h"
#include "ui_filetreeleftpane.h"

#include <QFileSystemModel>
#include <twindowtabberbutton.h>

struct FileTreeLeftPanePrivate {
        tWindowTabberButton* tabButton;
        ProjectPtr project;

        QFileSystemModel* model;
};

FileTreeLeftPane::FileTreeLeftPane(ProjectPtr project, QWidget* parent) :
    AbstractLeftPane(parent),
    ui(new Ui::FileTreeLeftPane) {
    ui->setupUi(this);
    d = new FileTreeLeftPanePrivate();
    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText(tr("Files"));

    d->project = project;

    d->model = new QFileSystemModel(this);
    d->model->setRootPath(d->project->projectDir().path());
    d->model->setFilter(static_cast<QDir::Filters>(QDir::Files | QDir::Dirs | QDir::DirsFirst | QDir::Hidden | QDir::NoDotAndDotDot));
    ui->fileSystemView->setModel(d->model);
    ui->fileSystemView->setRootIndex(d->model->index(d->project->projectDir().path()));
    ui->fileSystemView->setColumnHidden(1, true);
    ui->fileSystemView->setColumnHidden(2, true);
    ui->fileSystemView->setColumnHidden(3, true);
}

FileTreeLeftPane::~FileTreeLeftPane() {
    delete d;
    delete ui;
}

tWindowTabberButton* FileTreeLeftPane::tabButton() {
    return d->tabButton;
}

void FileTreeLeftPane::on_fileSystemView_clicked(const QModelIndex& index) {
    emit requestFileOpen(QUrl::fromLocalFile(index.data(QFileSystemModel::FilePathRole).toString()));
}
