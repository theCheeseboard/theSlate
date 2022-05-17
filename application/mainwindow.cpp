#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tapplication.h>
#include <tcsdtools.h>
#include <thelpmenu.h>
#include <tjobmanager.h>
#include <tpopover.h>
#include <twindowtabberbutton.h>

#include "pages/editorpage/editorpage.h"

struct MainWindowPrivate {
        tCsdTools csd;
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

    tHelpMenu* helpMenu = new tHelpMenu(this);

    menu->addMenu(helpMenu);
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

void MainWindow::on_actionExit_triggered() {
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
}
