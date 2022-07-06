#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QCoroFuture>
#include <QFileDialog>
#include <QStandardPaths>
#include <editormanager.h>
#include <editors/abstracteditor/abstracteditor.h>
#include <statemanager.h>
#include <tapplication.h>
#include <tcsdtools.h>
#include <thelpmenu.h>
#include <tjobmanager.h>
#include <tpopover.h>
#include <twindowtabberbutton.h>

#include "pages/editorpage/editorpage.h"
#include "pages/projectpage/projectpage.h"
#include "pages/repositoryclonepage/repositoryclonepage.h"
#include "unsavedchangespopover.h"

#include <objects/repository.h>
#include <popovers/clonerepositorypopover.h>

struct MainWindowPrivate {
        tCsdTools csd;

        bool forceClose = false;
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    d = new MainWindowPrivate();
    d->csd.installMoveAction(ui->topWidget);
    d->csd.installResizeAction(this);

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    } else {
        ui->rightCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    }

    ui->jobButtonLayout->addWidget(tJobManager::makeJobButton());

    this->resize(SC_DPI_WT(this->size(), QSize, this));

#ifdef Q_OS_MAC
    ui->menubar->addMenu(new tHelpMenu(this));
    ui->menuButton->setVisible(false);
#else
    ui->menubar->setVisible(false);
    QMenu* menu = new QMenu(this);
    menu->addMenu(ui->menuNew);
    menu->addMenu(ui->menuOpen);
    menu->addAction(ui->actionSave);
    menu->addAction(ui->actionSave_As);
    menu->addAction(ui->actionSave_All);
    menu->addSeparator();
    menu->addMenu(new tHelpMenu(this));
    menu->addAction(ui->actionExit);

    ui->menuButton->setIcon(tApplication::applicationIcon());
    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    ui->menuButton->setMenu(menu);
#endif

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    this->setWindowIcon(tApplication::applicationIcon());
}

MainWindow::~MainWindow() {
    //    d->csd.removeResizeAction(this);
    delete d;
    delete ui;
}

QCoro::Task<> MainWindow::tryClose() {
    QList<AbstractPage*> unsavedChangesPages;
    for (int i = 0; i < ui->stackedWidget->count(); i++) {
        AbstractPage* page = qobject_cast<AbstractPage*>(ui->stackedWidget->widget(i));
        if (page->saveAndCloseShouldAskUserConfirmation()) unsavedChangesPages.append(page);
    }

    if (unsavedChangesPages.isEmpty()) {
        d->forceClose = true;
        this->close();
        co_return;
    }

    UnsavedChangesPopover* jp = new UnsavedChangesPopover(unsavedChangesPages);
    std::function<void()> showPopover = [=] {
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI_W(-200, this));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &UnsavedChangesPopover::accepted, popover, &tPopover::dismiss);
        connect(jp, &UnsavedChangesPopover::rejected, popover, &tPopover::dismiss);
        connect(jp, &UnsavedChangesPopover::hide, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, popover, [=] {
            jp->setParent(nullptr);
            popover->deleteLater();
        });
        popover->show(this->window());
    };

    QPromise<bool>* promise = new QPromise<bool>();

    connect(jp, &UnsavedChangesPopover::rejected, this, [=] {
        jp->deleteLater();
        promise->start();
        promise->addResult(false);
        promise->finish();
        delete promise;
    });
    connect(jp, &UnsavedChangesPopover::accepted, this, [=] {
        d->forceClose = true;
        this->close();
        promise->start();
        promise->addResult(true);
        promise->finish();
        delete promise;
    });
    connect(jp, &UnsavedChangesPopover::show, this, showPopover);

    showPopover();

    auto success = co_await promise->future();
    if (!success) throw QException();
}

QCoro::Task<> MainWindow::on_actionExit_triggered() {
    QQueue<MainWindow*> mainWindows;
    for (auto widget : QApplication::topLevelWidgets()) {
        auto mw = qobject_cast<MainWindow*>(widget);
        if (mw) mainWindows.append(mw);
    }

    while (!mainWindows.isEmpty()) {
        auto window = mainWindows.dequeue();
        try {
            co_await window->tryClose();
        } catch (QException& ex) {
            co_return;
        }
    }

    // We've closed everything
    QApplication::exit();
}

void MainWindow::on_actionEmpty_Text_File_triggered() {
    EditorPage* editor = new EditorPage();
    this->addPage(editor);
}

void MainWindow::addPage(AbstractPage* page) {
    ui->stackedWidget->addWidget(page);
    ui->windowTabber->addButton(page->tabButton());
    page->tabButton()->syncWithStackedWidget(ui->stackedWidget, page);

    connect(page, &AbstractPage::done, this, [=] {
        ui->stackedWidget->removeWidget(page);
        ui->windowTabber->removeButton(page->tabButton());
        page->deleteLater();
    });
}

void MainWindow::on_actionUndo_triggered() {
    AbstractPage* currentPage = qobject_cast<AbstractPage*>(ui->stackedWidget->currentWidget());
    currentPage->undo();
}

void MainWindow::on_actionRedo_triggered() {
    AbstractPage* currentPage = qobject_cast<AbstractPage*>(ui->stackedWidget->currentWidget());
    currentPage->redo();
}

void MainWindow::on_actionOpenFile_triggered() {
    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    connect(fileDialog, &QFileDialog::finished, this, [=](int result) {
        if (result == QFileDialog::Accepted) {
            for (auto file : fileDialog->selectedFiles()) {
                QUrl fileUrl = QUrl::fromLocalFile(file);
                QString fileType = StateManager::editor()->editorTypeForUrl(fileUrl);

                EditorPage* editor = new EditorPage(fileType);
                this->addPage(editor);
                editor->discardContentsAndOpenFile(fileUrl);
            }
        }
    });
    connect(fileDialog, &QFileDialog::finished, fileDialog, &QFileDialog::deleteLater);
    fileDialog->open();
}

void MainWindow::on_actionSave_triggered() {
    AbstractPage* currentPage = qobject_cast<AbstractPage*>(ui->stackedWidget->currentWidget());
    currentPage->save();
}

void MainWindow::on_actionSave_As_triggered() {
    AbstractPage* currentPage = qobject_cast<AbstractPage*>(ui->stackedWidget->currentWidget());
    currentPage->saveAs();
}

void MainWindow::on_actionSave_All_triggered() {
    for (int i = 0; i < ui->stackedWidget->count(); i++) {
        AbstractPage* page = qobject_cast<AbstractPage*>(ui->stackedWidget->widget(i));
        page->saveAll();
    }
}

void MainWindow::on_actionClose_Tab_triggered() {
    AbstractPage* currentPage = qobject_cast<AbstractPage*>(ui->stackedWidget->currentWidget());
    currentPage->saveAndClose(false);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (d->forceClose) {
        this->deleteLater();
        return;
    }

    event->ignore();
    this->tryClose();
}

void MainWindow::on_actionOpenDirectory_triggered() {
    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    connect(fileDialog, &QFileDialog::finished, this, [=](int result) {
        if (result == QFileDialog::Accepted) {
            auto dir = fileDialog->selectedFiles().first();

            auto* project = new ProjectPage(dir);
            this->addPage(project);
        }
    });
    connect(fileDialog, &QFileDialog::finished, fileDialog, &QFileDialog::deleteLater);
    fileDialog->open();
}

void MainWindow::on_actionClone_Repository_triggered() {
    auto* jp = new CloneRepositoryPopover();
    auto* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI_W(-200, this));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &CloneRepositoryPopover::done, popover, &tPopover::dismiss);
    connect(jp, &CloneRepositoryPopover::openRepository, this, [=](RepositoryPtr repository) {
        auto* page = new RepositoryClonePage(repository);
        this->addPage(page);
    });
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &CloneRepositoryPopover::deleteLater);
    popover->show(this->window());
}
