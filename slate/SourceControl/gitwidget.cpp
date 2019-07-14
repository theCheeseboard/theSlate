#include "gitwidget.h"
#include "ui_gitwidget.h"

#include "gitintegration.h"
#include "commitsmodel.h"
#include "branchesmodel.h"
#include "GitDialogs/addbranchdialog.h"
#include "GitDialogs/commitdialog.h"
#include "GitDialogs/progressdialog.h"
#include "GitDialogs/authenticationdialog.h"
#include <tpopover.h>
#include <tmessagebox.h>
#include <QClipboard>
#include "mainwindow.h"

#include <QScroller>

struct GitWidgetPrivate {
    GitIntegration* gi;

    CommitsModel* commitsModel;
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

    d->commitsModel = new CommitsModel(d->gi);

    ui->logList->setModel(d->commitsModel);
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

    QScroller::grabGesture(ui->logList, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->branchesList, QScroller::LeftMouseButtonGesture);
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
                merge(branch->name);
            });
            menu->addAction(tr("Merge %1 into %2").arg(currentBranch->name, branch->name), [=] {
                d->gi->checkout(branch->name);
                merge(currentBranch->name);
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

            tPopover* p = new tPopover(dialog);
            p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
            connect(p, &tPopover::dismissed, [=] {
                p->deleteLater();
                dialog->deleteLater();
            });
            connect(dialog, &AddBranchDialog::accepted, p, [=] {
                d->gi->newBranch(dialog->name(), dialog->from());
                p->dismiss();
            });
            connect(dialog, &AddBranchDialog::rejected, p, [=] {
                p->dismiss();
            });
            p->show(this->window());
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
            menu->addAction(tr("Push"), [=] {
                this->push();
            });
            menu->addAction(tr("Pull"), [=] {
                this->pull();
            });
            menu->addSeparator();
            menu->addAction(tr("Fetch"), [=] {
                d->gi->fetch();
            });
            menu->exec(ui->branchesList->mapToGlobal(pos));
        }
    } else {
        //This is a commit
        GitIntegration::CommitPointer commit = index.data(Qt::UserRole).value<GitIntegration::CommitPointer>();
        QMenu* menu = new QMenu();
        menu->addSection(tr("For commit %1").arg(commit->hash.left(7)));
        menu->addAction(tr("Copy Identifier"), [=] {
            QApplication::clipboard()->setText(commit->hash);
        });
        menu->addAction(tr("Copy Commit Message"), [=] {
            QApplication::clipboard()->setText(commit->message);
        });
        menu->addAction(tr("Checkout"), [=] {
            d->gi->checkout(commit->hash);
        });

        auto performReset = [=](GitIntegration::ResetType type) {
            //Abort merge
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Reset the repository?"));

            if (type == GitIntegration::Hard) {
                messageBox->setText(tr("The changes since %1 will be discarded.").arg(commit->hash.left(7)));
            } else {
                messageBox->setText(tr("Your HEAD will point to %1.").arg(commit->hash.left(7)));
            }

            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(tMessageBox::Yes | tMessageBox::No);
            if (messageBox->exec() == tMessageBox::Yes) {
                d->gi->resetTo(commit->hash, type);
            }
            messageBox->deleteLater();
        };

        QMenu* resetMenu = new QMenu();
        resetMenu->setTitle(tr("Reset to here"));
        resetMenu->addAction(tr("Hard Reset"), [=] {
            performReset(GitIntegration::Hard);
        });
        resetMenu->addAction(tr("Mixed Reset"), [=] {
            performReset(GitIntegration::Mixed);
        });
        resetMenu->addAction(tr("Soft Reset"), [=] {
            performReset(GitIntegration::Soft);
        });
        menu->addMenu(resetMenu);

        menu->exec(ui->branchesList->mapToGlobal(pos));
    }
}

void GitWidget::on_logList_activated(const QModelIndex &index)
{
    if (!index.data(Qt::UserRole + 1).isNull()) {
        //This is an action
        QString action = index.data(Qt::UserRole + 1).toString();
        if (action == "new") {
            //Create a new commit
            commit();
        } else if (action == "pull") {
            //Pull from upstream
            pull();
        } else if (action == "push") {
            //Push to upstream
            push();
        } else if (action == "merge-abort") {
            //Abort merge
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Abort Merge?"));
            messageBox->setText(tr("Any actions taken to resolve conflict resolution will be undone, and the state of your repository will be set back to how it was before the merge operation started."));
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(tMessageBox::Yes | tMessageBox::No);
            if (messageBox->exec() == tMessageBox::Yes) {
                d->gi->abortMerge();
                d->commitsModel->reloadActions();
            }
            messageBox->deleteLater();
        }
    }
}

void GitWidget::commit() {
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
}

void GitWidget::pull(QString from) {
    if (!d->gi->status().trimmed().isEmpty()) {
        tMessageBox* messageBox = new tMessageBox(this->window());
        messageBox->setWindowTitle(tr("Unclean Working Directory"));
        messageBox->setText(tr("Your working directory is not clean and upstream changes may not merge properly. Do you still want to attempt to pull in upstream changes?"));
        messageBox->setIcon(tMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        QPushButton* discardAll = messageBox->addButton(tr("Discard All Changes and Pull"), tMessageBox::DestructiveRole);
        messageBox->addButton(tr("Pull Anyway"), tMessageBox::AcceptRole);
        messageBox->addButton(tMessageBox::Cancel);

        connect(discardAll, &QPushButton::clicked, discardAll, [=] {
            //Discard
            d->gi->resetAll();
        });

        int button = messageBox->exec();
        messageBox->deleteLater();
        if (button == QMessageBox::Cancel) {
            //Stop here
            return;
        }
    }

    QString pullRepo = from;
    if (pullRepo == "") pullRepo = d->gi->branch()->upstream->name;

    //Show a dialog
    ProgressDialog* dialog = new ProgressDialog();
    dialog->setTitle(tr("Pull"));
    dialog->setMessage(tr("Pulling from %1...").arg(pullRepo));
    dialog->setCancelable(false);
    dialog->resize(dialog->sizeHint());

    tPopover* p = new tPopover(dialog);
    p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    p->setDismissable(false);
    connect(p, &tPopover::dismissed, [=] {
        p->deleteLater();
        dialog->deleteLater();
    });
    p->show(this->window());

    //Pull in everything
    d->gi->pull(from)->then([=] {
        p->dismiss();
    })->error([=](QString error) {
        p->dismiss();
        if (error == "unclean") {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Unclean Working Directory"));
            messageBox->setText(tr("Commit or discard changes in your working directory in order to pull."));
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->exec();
            messageBox->deleteLater();
        } else if (error == "conflicting") {
            showConflictDialog(tr("pull"));
        } else if (error == "authenticate") {
            setAuthenticationDetails(tr("Authenticate to pull from %1").arg(pullRepo), [=] {
                pull(from);
            });
        } else {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Git Error"));
            messageBox->setText(error);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
        }
    });
}

void GitWidget::push(QString to) {
    QString pushRepo = to;
    if (pushRepo == "") pushRepo = d->gi->branch()->upstream->name;

    //Show a dialog
    ProgressDialog* dialog = new ProgressDialog();
    dialog->setTitle(tr("Push"));
    dialog->setMessage(tr("Pushing to %1...").arg(pushRepo));
    dialog->setCancelable(false);
    dialog->resize(dialog->sizeHint());

    tPopover* p = new tPopover(dialog);
    p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    p->setDismissable(false);
    connect(p, &tPopover::dismissed, [=] {
        p->deleteLater();
        dialog->deleteLater();
    });
    p->show(this->window());

    //Perform the push
    d->gi->push(to)->then([=] {
        p->dismiss();
    })->error([=](QString error) {
        p->dismiss();
        if (error == "out-of-date") {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Out of date"));
            messageBox->setText(tr("Your local repository is out of date and needs to be updated by pulling."));
            QPushButton* pullButton = messageBox->addButton(tr("Pull"), tMessageBox::DestructiveRole);
            messageBox->addButton(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);

            connect(pullButton, &QPushButton::clicked, [=] {
                pull(to);
            });

            messageBox->exec();
            messageBox->deleteLater();
        } else if (error.startsWith("message")) {
            QStringList parts = error.split("\n");

            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(parts.at(1));
            messageBox->setText(parts.mid(2).join("\n"));
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
        } else if (error == "authenticate") {
            setAuthenticationDetails(tr("Authenticate to push to %1").arg(pushRepo), [=] {
                push(to);
            });
        } else {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Git Error"));
            messageBox->setText(error);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
        }
    });
}

void GitWidget::setAuthenticationDetails(QString message, std::function<void()> callback) {
    //Show a dialog
    AuthenticationDialog* dialog = new AuthenticationDialog();
    dialog->setMessage(message);
    dialog->resize(300 * theLibsGlobal::getDPIScaling(), dialog->sizeHint().height());

    QEventLoop* loop = new QEventLoop();
    bool* cont = new bool(false);

    tPopover* p = new tPopover(dialog);
    p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    p->setDismissable(false);
    connect(p, &tPopover::dismissed, p, [=] {
        loop->quit();
    });
    connect(dialog, &AuthenticationDialog::rejected, p, [=] {
        p->dismiss();
    });
    connect(dialog, &AuthenticationDialog::accepted, p, [=] {
        *cont = true;
        p->dismiss();
    });
    p->show(this->window());

    loop->exec();
    loop->deleteLater();

    if (*cont) {
        d->gi->setNextCredentials(dialog->username(), dialog->password());
        callback();
    }
    delete cont;
}

void GitWidget::showConflictDialog(QString operation) {
    //Find the files that are conflicting
    QStringList conflictingFiles;
    QList<QByteArray> files = d->gi->status().split('\0');
    for (QByteArray file : files) {
        if (file.isEmpty()) continue;
        QString status = file.left(2);
        QString filename = file.mid(3);

        if (status.contains("U") || status == "AA" || status == "DD") {
            conflictingFiles.append(filename);
        }
    }

    tMessageBox* messageBox = new tMessageBox(this->window());
    messageBox->setWindowTitle(tr("Conflicting Files"));
    messageBox->setText(tr("The %1 operation resulted in these files conflicting:").arg(operation) + "\n" + conflictingFiles.join("\n") + "\n\n" + tr("What do you want to do now?"));
    messageBox->setIcon(tMessageBox::Warning);
    messageBox->setWindowFlags(Qt::Sheet);
    QPushButton* undoButton = messageBox->addButton(tr("Undo %1").arg(operation), tMessageBox::DestructiveRole);
    QPushButton* resolveButton = messageBox->addButton(tr("Manually Resolve Changes"), tMessageBox::AcceptRole);
    QPushButton* localButton = messageBox->addButton(tr("Use Local Changes"), tMessageBox::YesRole);
    QPushButton* remoteButton = messageBox->addButton(tr("Use Remote Changes"), tMessageBox::NoRole);

    connect(undoButton, &QPushButton::clicked, undoButton, [=] {
        //Abort Merge
        d->gi->abortMerge();
    });
    connect(localButton, &QPushButton::clicked, localButton, [=] {
        //Checkout all of our files
        for (QString file : conflictingFiles) {
            d->gi->checkout(file, "--ours");
            d->gi->add(file);
        }

        //Commit the result
        d->gi->commit(d->gi->defaultCommitMessage());
    });
    connect(remoteButton, &QPushButton::clicked, remoteButton, [=] {
        //Checkout all of their files
        for (QString file : conflictingFiles) {
            d->gi->checkout(file, "--theirs");
            d->gi->add(file);
        }

        //Commit the result
        d->gi->commit(d->gi->defaultCommitMessage());
    });

    messageBox->exec();
    messageBox->deleteLater();

    d->commitsModel->reloadActions();
}

void GitWidget::merge(QString other) {
    //Pull in everything
    d->gi->merge(other)->then([=] {

    })->error([=](QString error) {
        if (error == "conflicting") {
            showConflictDialog(tr("merge"));
        } else if (error == "unrelated") {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Unrelated Histories"));
            messageBox->setText(tr("You're trying to merge two branches which do not have a common base"));
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
        } else {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Git Error"));
            messageBox->setText(error);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
        }
    });
}
