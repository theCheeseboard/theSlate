#include "commitdialog.h"
#include "ui_commitdialog.h"

#include "../statusmodel.h"
#include "texteditor.h"
#include "plugins/pluginmanager.h"
#include <tmessagebox.h>

struct CommitDialogPrivate {
    GitIntegration* gi;
    StatusModel* model;

    TextEditor *side1, *side2;
};

extern PluginManager* plugins;
CommitDialog::CommitDialog(GitIntegration* integration, MainWindow* mainWin, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommitDialog)
{
    ui->setupUi(this);
    d = new CommitDialogPrivate();
    d->gi = integration;

#ifdef Q_OS_MAC
    ui->cancelButton->setVisible(false);
    ui->acceptButton->setVisible(false);
#else
    ui->windowControlsMac->setVisible(false);
#endif

    QPalette pal = ui->branchChunk->palette();
    pal.setColor(QPalette::Window, QColor(0, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->branchChunk->setPalette(pal);
    pal.setColor(QPalette::Window, QColor(0, 100, 255));
    ui->userChunk->setPalette(pal);

    ui->branchIcon->setPixmap(QIcon::fromTheme("branch", QIcon(":/icons/branch.svg")).pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
    ui->userIcon->setPixmap(QIcon::fromTheme("user", QIcon(":/icons/user.svg")).pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));

    if (d->gi->branch().isNull()) {
        ui->currentBranch->setText("...");
    } else {
        ui->currentBranch->setText(this->fontMetrics().elidedText(d->gi->branch()->name, Qt::ElideRight, 200 * theLibsGlobal::getDPIScaling()));
    }

    ui->currentUser->setText(d->gi->myName());

    d->model = new StatusModel(d->gi);

    if (d->gi->isConflicting()) {
        ui->titleLabel->setText(tr("Conclude Merge"));
        ui->messageLabel->setText(tr("Review your resolutions and complete the ongoing merge"));
        ui->macCommitButton->setText(tr("Merge"));

        ui->commitMessage->setPlainText(d->gi->defaultCommitMessage());
    }

    ui->filesView->setModel(d->model);
    ui->filesView->setItemDelegate(new StatusModelDelegate(d->gi));
    ui->leftPane->setFixedWidth(300 * theLibsGlobal::getDPIScaling());

    //ui->commitMessage->setPlainText(d->gi->status());

    d->side1 = new TextEditor(mainWin);
    d->side2 = new TextEditor(mainWin);

    d->side1->lockScrolling(d->side2);
    d->side1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->side1->setReadOnly(true);
    d->side2->setReadOnly(true);

    d->side1->setFrameStyle(QFrame::NoFrame);
    d->side2->setFrameStyle(QFrame::NoFrame);

    ui->mergeToolsLayout->addWidget(d->side1);
    ui->mergeToolsLayout->addWidget(d->side2);

    connect(ui->filesView->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](QModelIndex current, QModelIndex previous) {
        if (current.isValid()) {
            QString filename = current.data(Qt::DisplayRole).toString();

            StatusModel::Status s = current.data(Qt::UserRole + 1).value<StatusModel::Status>();
            if (s.status.at(1) == 'D' || s.status == "D ") {
                d->side1->openFileFake(tr("File deleted from disk"));
            } else {
                d->side1->openFile(plugins->getLocalFileBackend()->openFromUrl(QUrl::fromLocalFile(d->gi->rootDir() + "/" + filename)));
            }

            if (s.status.at(0) == 'A' || s.status.at(0) == '?') {
                d->side2->openFileFake(tr("New file"));
            } else {
                d->side2->openFileFake(d->gi->show("HEAD:" + filename));
            }
        }
    });
}

CommitDialog::~CommitDialog()
{
    delete d;
    delete ui;
}

void CommitDialog::on_acceptButton_clicked()
{
    //Perform sanity checks

    bool haveCommittedFiles = false;
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i);
        if (index.data(Qt::CheckStateRole).toBool()) {
            haveCommittedFiles = true;
            break;
        }
    }

    if (!haveCommittedFiles) {
        tMessageBox* b = new tMessageBox(this->window());
        b->setWindowTitle(tr("Nothing to commit"));
        b->setText(tr("You haven't added any files to this commit."));
        b->setIcon(tMessageBox::Warning);
        b->setWindowFlags(Qt::Sheet);
        b->setStandardButtons(tMessageBox::Ok);
        b->setDefaultButton(tMessageBox::Ok);
        b->exec();
        b->deleteLater();
        return;
    }

    if (ui->commitMessage->toPlainText().isEmpty()) {
        tMessageBox* b = new tMessageBox(this->window());
        b->setWindowTitle(tr("No Commit Message"));
        b->setText(tr("You'll need to set a commit message to commit these files."));
        b->setIcon(tMessageBox::Warning);
        b->setWindowFlags(Qt::Sheet);
        b->setStandardButtons(tMessageBox::Ok);
        b->setDefaultButton(tMessageBox::Ok);
        b->exec();
        b->deleteLater();
        return;
    }

    if (!d->gi->isConflicting()) {
        //Reset all staged changes to prepare for commit
        d->gi->reset();
    }

    //Add each change
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i);
        if (index.data(Qt::CheckStateRole).toBool()) {
            //Add this file to the index
            d->gi->add(index.data(Qt::DisplayRole).toString());
        }
    }

    //Commit everything
    d->gi->commit(ui->commitMessage->toPlainText());

    //Finish the commit
    this->accept();
}
