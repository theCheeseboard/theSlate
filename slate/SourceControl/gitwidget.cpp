#include "gitwidget.h"
#include "ui_gitwidget.h"

#include "gitintegration.h"
#include "commitsmodel.h"
#include "branchesmodel.h"
#include "GitDialogs/addbranchdialog.h"
#include "GitDialogs/commitdialog.h"
#include <tpopover.h>
#include <tmessagebox.h>
#include "mainwindow.h"

struct GitWidgetPrivate {
    GitIntegration* gi;
};

GitWidget::GitWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GitWidget)
{
    ui->setupUi(this);
    d = new GitWidgetPrivate();

    QPalette pal = ui->branchChunk->palette();
    pal.setColor(QPalette::Window, QColor(0, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->branchChunk->setPalette(pal);
    pal.setColor(QPalette::Window, QColor(0, 150, 0));
    ui->commitChunk->setPalette(pal);

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
    ui->branchesList->setModel(new BranchesModel(d->gi));
    ui->branchesList->setItemDelegate(new BranchesModelDelegate(d->gi));

    connect(d->gi, &GitIntegration::headCommitChanged, [=] {
        GitIntegration::CommitPointer commit = d->gi->getCommit("HEAD", false, false);
        if (commit.isNull()) {
            ui->currentCommit->setText("...");
        } else {
            ui->currentCommit->setText(commit->hash.left(7));
        }
    });
    connect(d->gi, &GitIntegration::currentBranchChanged, [=] {
        if (d->gi->branch().isNull()) {
            ui->currentBranch->setText("...");
        } else {
            ui->currentBranch->setText(this->fontMetrics().elidedText(d->gi->branch()->name, Qt::ElideRight, 200 * theLibsGlobal::getDPIScaling()));
        }
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

void GitWidget::on_branchesButton_toggled(bool checked)
{
    if (checked) ui->gitPages->setCurrentIndex(0);
}

void GitWidget::on_commitsButton_toggled(bool checked)
{
    if (checked) ui->gitPages->setCurrentIndex(1);
}

void GitWidget::on_branchesList_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->branchesList->indexAt(pos);
    if (index.data(Qt::UserRole + 1).isNull()) {
        //This is an action
    } else {
        GitIntegration::BranchPointer branch = index.data(Qt::UserRole + 1).value<GitIntegration::BranchPointer>();
        GitIntegration::BranchPointer currentBranch = d->gi->branch();
        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(branch->name));
        menu->addAction(tr("Checkout"), [=] {
            d->gi->checkout(branch->name);
        });
        QAction* deleteAction = menu->addAction(tr("Delete"), [=] {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Delete Branch"));
            messageBox->setText(tr("Delete the %1 branch?").arg(branch->name));
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            messageBox->setDefaultButton(QMessageBox::No);
            if (messageBox->exec() == QMessageBox::Yes) {
                //Delete the branch
                d->gi->deleteBranch(branch);
            }
        });
        if (branch == currentBranch) deleteAction->setEnabled(false);

        if (currentBranch != branch) {
            menu->addSection(tr("With %1").arg(currentBranch->name));
            menu->addAction(tr("Merge %1 into %2").arg(branch->name, currentBranch->name), [=] {

            });
            menu->addAction(tr("Merge %1 into %2").arg(currentBranch->name, branch->name), [=] {

            });
        }
        menu->exec(ui->branchesList->mapToGlobal(pos));
    }
}

void GitWidget::on_branchesList_activated(const QModelIndex &index)
{
    if (index.data(Qt::UserRole + 1).isNull()) {
        //This is an action
        QString action = index.data(Qt::UserRole).toString();
        if (action == "new") {
            //Create a new branch
            AddBranchDialog* dialog = new AddBranchDialog(d->gi, this->window());
            dialog->setWindowFlags(Qt::Sheet);
            if (dialog->exec() == AddBranchDialog::Accepted) {
                //Make the branch
                d->gi->newBranch(dialog->name(), dialog->from());
            }
        }
    }
}

void GitWidget::on_logList_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->logList->indexAt(pos);
    if (!index.data(Qt::UserRole + 1).isNull()) {
        //This is an action
        QString action = index.data(Qt::UserRole + 1).toString();
        if (action == "pull" || action == "push" || action == "up-to-date") {
            QMenu* menu = new QMenu();
            menu->addSection(tr("For repository"));
            menu->addAction(tr("Push"));
            menu->addAction(tr("Pull"));
            menu->addSeparator();
            menu->addAction(tr("Fetch"));
            menu->exec(ui->branchesList->mapToGlobal(pos));
        }
    }
}

void GitWidget::on_logList_activated(const QModelIndex &index)
{
    if (!index.data(Qt::UserRole + 1).isNull()) {
        //This is an action
        QString action = index.data(Qt::UserRole + 1).toString();
        if (action == "new") {
            //Create a new commit
            CommitDialog* dialog = new CommitDialog(d->gi, static_cast<MainWindow*>(this->window()), this->window());
            dialog->resize(this->window()->width() - 20, this->window()->height() - 50);

            tPopover* p = new tPopover(dialog);
            p->setPopoverWidth(-100 * theLibsGlobal::getDPIScaling());
            connect(dialog, &CommitDialog::finished, [=] {
                p->dismiss();
            });
            connect(p, &tPopover::dismissed, [=] {
                p->deleteLater();
                dialog->deleteLater();
            });
            p->show(this->window());
            dialog->activateWindow();
        }
    }
}
