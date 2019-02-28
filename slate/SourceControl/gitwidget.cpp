#include "gitwidget.h"
#include "ui_gitwidget.h"

#include "gitintegration.h"
#include "commitsmodel.h"

struct GitWidgetPrivate {
    GitIntegration* gi;
};

GitWidget::GitWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GitWidget)
{
    ui->setupUi(this);
    d = new GitWidgetPrivate();

    ui->branchIcon->setPixmap(QIcon::fromTheme("branch", QIcon(":/icons/branch.svg")).pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
    ui->commitIcon->setPixmap(QIcon::fromTheme("commit", QIcon(":/icons/commit.svg")).pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));

    d->gi = new GitIntegration("");
    setCurrentDocument(QUrl());
    if (GitIntegration::findGit().count() == 0) {
        //Tell everyone Git isn't found
        ui->mainStack->setCurrentIndex(2);
    }

    ui->logList->setModel(new CommitsModel(d->gi));
    ui->logList->setItemDelegate(new CommitsModelDelegate(d->gi));

    connect(d->gi, &GitIntegration::commitsChanged, [=] {
        ui->currentCommit->setText(d->gi->getCommit("HEAD", false, false)->hash.left(7));
    });
}

GitWidget::~GitWidget()
{
    delete ui;
    delete d;
}

void GitWidget::setCurrentDocument(QUrl currentDocument) {
    if (currentDocument.isEmpty() || !currentDocument.isLocalFile()) {
        ui->mainStack->setCurrentIndex(1);
    } else {
        if (d->gi->setNewRootDir(QFileInfo(currentDocument.toLocalFile()).dir().path())) {
            if (d->gi->needsInit()) {
                ui->mainStack->setCurrentIndex(0);
            } else {
                ui->mainStack->setCurrentIndex(3);
                updateStatus();
            }
        } else {
            ui->mainStack->setCurrentIndex(3);
        }
    }
}

void GitWidget::updateStatus() {

}
