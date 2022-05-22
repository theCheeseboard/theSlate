#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
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
#include "unsavedchangespopover.h"

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
    delete ui;
}

tPromise<void>* MainWindow::tryClose() {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        QList<AbstractPage*> unsavedChangesPages;
        for (int i = 0; i < ui->stackedWidget->count(); i++) {
            AbstractPage* page = qobject_cast<AbstractPage*>(ui->stackedWidget->widget(i));
            if (page->saveAndCloseShouldAskUserConfirmation()) unsavedChangesPages.append(page);
        }

        if (unsavedChangesPages.isEmpty()) {
            d->forceClose = true;
            this->close();
            res();
            return;
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

        connect(jp, &UnsavedChangesPopover::rejected, this, [=] {
            rej("User cancelled");
            jp->deleteLater();
        });
        connect(jp, &UnsavedChangesPopover::accepted, this, [=] {
            d->forceClose = true;
            this->close();
            res();
            jp->deleteLater();
        });
        connect(jp, &UnsavedChangesPopover::show, this, showPopover);

        showPopover();
    });
}

void MainWindow::on_actionExit_triggered() {
    QQueue<MainWindow*> mainWindows;
    for (auto widget : QApplication::topLevelWidgets()) {
        auto mw = qobject_cast<MainWindow*>(widget);
        if (mw) mainWindows.append(mw);
    }

    std::function<void(QQueue<MainWindow*>)> closeNextWindow = [&closeNextWindow](QQueue<MainWindow*> mainWindows) {
        // We've closed everything
        if (mainWindows.isEmpty()) {
            QApplication::exit();
            return;
        }

        MainWindow* mw = mainWindows.dequeue();
        mw->tryClose()->then([=] {
            closeNextWindow(mainWindows);
        });
    };
    closeNextWindow(mainWindows);
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
    connect(fileDialog, &QFileDialog::finished, this, [=](int result) {
        if (result == QFileDialog::Accepted) {
            for (auto file : fileDialog->selectedFiles()) {
                QString fileType = StateManager::editor()->editorTypeForFileName(file);

                EditorPage* editor = new EditorPage(fileType);
                this->addPage(editor);

                QFile f(file);
                f.open(QFile::ReadOnly);
                editor->editor()->setData(f.readAll());
                f.close();

                editor->editor()->setCurrentUrl(QUrl::fromLocalFile(file));
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
