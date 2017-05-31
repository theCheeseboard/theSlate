#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    //Set up special palette for Windows
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor(255, 255, 255));
    this->setPalette(pal);
    //ui->tabFrame->setPalette(pal);
#endif

    //Set up single menu
    QMenu* singleMenu = new QMenu();
    singleMenu->addAction(ui->actionNew);
    singleMenu->addSeparator();
    singleMenu->addAction(ui->actionOpen);
    singleMenu->addAction(ui->actionSave);
    singleMenu->addSeparator();
    singleMenu->addAction(ui->actionExit);

    QToolButton* menuButton = new QToolButton();
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setMenu(singleMenu);
    menuButton->setArrowType(Qt::NoArrow);
    menuButton->setIcon(QIcon::fromTheme("theslate", QIcon(":/icons/icon.svg")));
    ui->mainToolBar->insertWidget(ui->actionNew, menuButton);

    ui->menuBar->setVisible(false);

    newTab();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newTab() {
    TextEditor* view = new TextEditor();
    ui->tabs->addWidget(view);
    ui->tabs->setCurrentWidget(view);

    connect(view->getTabButton(), &QPushButton::clicked, [=]{
        ui->tabs->setCurrentWidget(view);
    });
    ui->tabButtons->addWidget(view->getTabButton());
}

void MainWindow::on_actionNew_triggered()
{
    newTab();
}

void MainWindow::on_tabs_currentChanged(int arg1)
{
    for (int i = 0; i < ui->tabs->count(); i++) {
        TextEditor* item = (TextEditor*) ui->tabs->widget(i);
        item->setActive(false);
    }

    ((TextEditor*) ui->tabs->widget(arg1))->setActive(true);
}
