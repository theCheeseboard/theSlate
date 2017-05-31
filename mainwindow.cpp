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

    #ifdef Q_OS_MACOS
        //Set up Mac toolbar
        ui->mainToolBar->setVisible(false);

        QMacToolBar* toolbar = new QMacToolBar();

        QList<QMacToolBarItem*> allowedItems;

        QMacToolBarItem* newItem = new QMacToolBarItem();
        newItem->setText("New Document");
        newItem->setIcon(QIcon(":/icons/document-new.svg"));
        newItem->setProperty("name", "new");
        connect(newItem, SIGNAL(activated()), this, SLOT(on_actionNew_triggered()));
        allowedItems.append(newItem);

        QMacToolBarItem* openItem = new QMacToolBarItem();
        openItem->setText("Open");
        openItem->setIcon(QIcon(":/icons/document-open.svg"));
        openItem->setProperty("name", "open");
        connect(openItem, SIGNAL(activated()), this, SLOT(on_actionOpen_triggered()));
        allowedItems.append(openItem);

        QMacToolBarItem* saveItem = new QMacToolBarItem();
        saveItem->setText("Save");
        saveItem->setIcon(QIcon(":/icons/document-save.svg"));
        saveItem->setProperty("name", "save");
        connect(saveItem, SIGNAL(activated()), this, SLOT(on_actionSave_triggered()));
        allowedItems.append(saveItem);

        toolbar->setAllowedItems(allowedItems);
        toolbar->setItems(allowedItems);

        toolbar->attachToWindow(this->windowHandle());
    #else
        //Set up single menu except on macOS
        QMenu* singleMenu = new QMenu();
        singleMenu->addAction(ui->actionNew);
        singleMenu->addSeparator();
        singleMenu->addAction(ui->actionOpen);
        singleMenu->addAction(ui->actionSave);
        singleMenu->addSeparator();
        singleMenu->addAction(ui->actionCut);
        singleMenu->addAction(ui->actionCopy);
        singleMenu->addAction(ui->actionPaste);
        singleMenu->addSeparator();
        singleMenu->addMenu(ui->menuCode);
        singleMenu->addSeparator();
        singleMenu->addAction(ui->actionAbout);
        singleMenu->addAction(ui->actionExit);

        QToolButton* menuButton = new QToolButton();
        menuButton->setPopupMode(QToolButton::InstantPopup);
        menuButton->setMenu(singleMenu);
        menuButton->setArrowType(Qt::NoArrow);
        menuButton->setIcon(QIcon::fromTheme("theslate", QIcon(":/icons/icon.svg")));
        ui->mainToolBar->insertWidget(ui->actionNew, menuButton);
    #endif

    ui->menuBar->setVisible(false);

    //Set up code highlighting options
    ui->menuCode->addAction("C++", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::cpp);});
    ui->menuCode->addAction("JavaScript", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::js);});

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

    connect(view, SIGNAL(editedChanged()), this, SLOT(checkForEdits()));
    connect(view->getTabButton(), &QPushButton::clicked, [=]{
        ui->tabs->setCurrentWidget(view);
    });
    connect(view, &TextEditor::fileNameChanged, [=] {
        if (currentDocument() == view) {
            this->setWindowFilePath(view->filename());
        }
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

    TextEditor* current = (TextEditor*) ui->tabs->widget(arg1);
    if (current != NULL) {
        current->setActive(true);
        this->setWindowFilePath(current->filename());
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::closeAllWindows();
}

void MainWindow::on_actionOpen_triggered()
{
    QEventLoop* loop = new QEventLoop();
    QFileDialog* openDialog = new QFileDialog(this, Qt::Sheet);
    openDialog->setWindowModality(Qt::WindowModal);
    openDialog->setAcceptMode(QFileDialog::AcceptOpen);
    openDialog->setDirectory(QDir::home());
    openDialog->setNameFilter("All Files (*)");
    connect(openDialog, SIGNAL(finished(int)), openDialog, SLOT(deleteLater()));
    connect(openDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
    openDialog->show();

    //Block until dialog is finished
    loop->exec();
    loop->deleteLater();

    if (openDialog->result() == QDialog::Accepted) {
        if (currentDocument()->isEdited() || currentDocument()->filename() != "") {
            newTab();
        }

        currentDocument()->openFile(openDialog->selectedFiles().first());
    }
}

void MainWindow::on_actionSave_triggered()
{
    saveCurrentDocument();
}

TextEditor* MainWindow::currentDocument() {
    return (TextEditor*) ui->tabs->widget(ui->tabs->currentIndex());
}

void MainWindow::checkForEdits() {
    for (int i = 0; i < ui->tabs->count(); i++) {
        TextEditor* item = (TextEditor*) ui->tabs->widget(i);
        if (item->isEdited()) {
            this->setWindowModified(true);
            return;
        }
    }
    this->setWindowModified(false);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    while (ui->tabs->count() > 0) {
        if (!closeCurrentTab()) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

bool MainWindow::saveCurrentDocument() {
    if (currentDocument()->filename() == "") {
        QEventLoop* loop = new QEventLoop();
        QFileDialog* saveDialog = new QFileDialog(this, Qt::Sheet);
        saveDialog->setWindowModality(Qt::WindowModal);
        saveDialog->setAcceptMode(QFileDialog::AcceptSave);
        saveDialog->setDirectory(QDir::home());
        saveDialog->setNameFilters(QStringList() << "Text File (*.txt)"
                                                 << "All Files (*)");
        connect(saveDialog, SIGNAL(finished(int)), saveDialog, SLOT(deleteLater()));
        connect(saveDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        saveDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (saveDialog->result() == QDialog::Accepted) {
            return currentDocument()->saveFile(saveDialog->selectedFiles().first());
        } else {
            return false;
        }
    } else {
        return currentDocument()->saveFile();
    }
}

bool MainWindow::closeCurrentTab() {
    if (currentDocument()->isEdited()) {
        QMessageBox* messageBox = new QMessageBox(this);
        messageBox->setWindowTitle("Save Changes?");
        messageBox->setText("Do you want to save your changes to this document?");
        messageBox->setIcon(QMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(QMessageBox::Discard | QMessageBox::Save | QMessageBox::Cancel);
        messageBox->setDefaultButton(QMessageBox::Save);
        int button = messageBox->exec();

        if (button == QMessageBox::Save) {
            if (!saveCurrentDocument()) {
                return false;
            }
        } else if (button == QMessageBox::Cancel) {
            return false;
        }
    }

    TextEditor* current = currentDocument();
    ui->tabButtons->removeWidget(current->getTabButton());
    ui->tabs->removeWidget(current);
    current->deleteLater();
    return true;
}


void MainWindow::on_closeButton_clicked()
{
    closeCurrentTab();
}

void MainWindow::on_actionCopy_triggered()
{
    currentDocument()->copy();
}

void MainWindow::on_actionCut_triggered()
{
    currentDocument()->cut();
}

void MainWindow::on_actionPaste_triggered()
{
    currentDocument()->paste();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow aboutWindow;
    aboutWindow.exec();
}

void MainWindow::on_actionNo_Highlighting_triggered()
{
    currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::none);
}
