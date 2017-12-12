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
        ui->toolBar->setVisible(false);
        ui->mainToolBar->setVisible(false);

        QMacToolBar* toolbar = new QMacToolBar();

        QList<QMacToolBarItem*> allowedItems;

        QMacToolBarItem* newItem = new QMacToolBarItem();
        newItem->setText(tr("New"));
        newItem->setIcon(QIcon(":/icons/document-new.svg"));
        newItem->setProperty("name", "new");
        connect(newItem, SIGNAL(activated()), this, SLOT(on_actionNew_triggered()));
        allowedItems.append(newItem);

        QMacToolBarItem* openItem = new QMacToolBarItem();
        openItem->setText(tr("Open"));
        openItem->setIcon(QIcon(":/icons/document-open.svg"));
        openItem->setProperty("name", "open");
        connect(openItem, SIGNAL(activated()), this, SLOT(on_actionOpen_triggered()));
        allowedItems.append(openItem);

        QMacToolBarItem* saveItem = new QMacToolBarItem();
        saveItem->setText(tr("Save"));
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
        singleMenu->addMenu(ui->menuNew);
        singleMenu->addAction(ui->actionOpen);
        singleMenu->addAction(ui->actionSave);
        singleMenu->addAction(ui->actionSave_All);
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

    //Hide the project frame
    ui->projectFrame->setVisible(false);
    ui->actionFile_in_Project->setVisible(false);
    ui->menuSource_Control->setEnabled(false);
    ui->actionStart->setVisible(false);
    ui->actionContinue->setVisible(false);
    ui->actionStep_Into->setVisible(false);
    ui->actionStep_Out->setVisible(false);
    ui->actionStep_Over->setVisible(false);
    ui->actionPause->setVisible(false);

    ui->sourceControlOptionsButton->setMenu(ui->menuSource_Control);

    //Set up code highlighting options
    ui->menuCode->addAction("C++", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::cpp);});
    ui->menuCode->addAction("JavaScript", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::js);});
    ui->menuCode->addAction("Python", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::py);});
    ui->menuCode->addAction("XML", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::xml);});
    ui->menuCode->addAction("Markdown", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::md);});
    ui->menuCode->addAction("JavaScript Object Notation (JSON)", [=] {setCurrentDocumentHighlighting(SyntaxHighlighter::json);});

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

    if (this->currentProjectFile != "") {
        view->enableIDEMode();
    }

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

    ui->closeButton->setVisible(true);
    ui->actionSave->setEnabled(true);
    ui->menuCode->setEnabled(true);
}

void MainWindow::on_actionNew_triggered()
{
    if (currentProjectFile == "") {
        newTab();
    } else {
        MainWindow* newWin = new MainWindow();
        newWin->show();
    }
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

    if (currentProjectFile == "") {
        openDialog->setDirectory(QDir::home());
    } else {
        openDialog->setDirectory(QFileInfo(currentProjectFile).path());
    }

    openDialog->setNameFilter("All Files (*)");
    connect(openDialog, SIGNAL(finished(int)), openDialog, SLOT(deleteLater()));
    connect(openDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
    openDialog->show();

    //Block until dialog is finished
    loop->exec();
    loop->deleteLater();

    if (openDialog->result() == QDialog::Accepted) {
        if (QFileInfo(openDialog->selectedFiles().first()).suffix() == "tslprj") {
            openProject(openDialog->selectedFiles().first());
        } else {
            if (currentDocument() == NULL || currentDocument()->isEdited() || currentDocument()->filename() != "") {
                newTab();
            }

            currentDocument()->openFile(openDialog->selectedFiles().first());
        }
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

        if (currentProjectFile == "") {
            saveDialog->setDirectory(QDir::home());
        } else {
            saveDialog->setDirectory(QFileInfo(currentProjectFile).path());
        }

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
        messageBox->setWindowTitle(tr("Save Changes?"));
        messageBox->setText(tr("Do you want to save your changes to this document?"));
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

    if (ui->tabs->count() == 0) {
        ui->closeButton->setVisible(false);
        ui->actionSave->setEnabled(false);
        ui->menuCode->setEnabled(false);
    }
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

void MainWindow::on_actionNew_theSlate_Project_triggered()
{
    //Query for filename and location
    QEventLoop* loop = new QEventLoop();
    QFileDialog* saveDialog = new QFileDialog(this, Qt::Sheet);
    saveDialog->setWindowModality(Qt::WindowModal);
    saveDialog->setAcceptMode(QFileDialog::AcceptSave);
    saveDialog->setDirectory(QDir::home());
    saveDialog->setNameFilters(QStringList() << "theSlate Project File (*.tslprj)");
    saveDialog->setDefaultSuffix("tslprj");
    connect(saveDialog, SIGNAL(finished(int)), saveDialog, SLOT(deleteLater()));
    connect(saveDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
    saveDialog->show();

    //Block until dialog is finished
    loop->exec();
    loop->deleteLater();

    if (saveDialog->result() == QDialog::Accepted) {
        //Initiate a project from files in this folder

        //QFile(":/initialStartupFile").copy(saveDialog->selectedFiles().first());
        QFile newFile(saveDialog->selectedFiles().first());
        QFile resourceFile(":/initialStartupFile");
        newFile.open(QFile::WriteOnly);
        resourceFile.open(QFile::ReadOnly);
        newFile.write(resourceFile.readAll());
        newFile.close();
        resourceFile.close();

        newTab();
        currentDocument()->openFile(saveDialog->selectedFiles().first());

        openProject(saveDialog->selectedFiles().first());
    }
}

void MainWindow::openProject(QString tslprjPath) {
    //Open project explorers
    ui->projectFrame->setVisible(true);
    ui->actionFile_in_Project->setVisible(true);
    ui->actionNew_theSlate_Project->setVisible(false);
    ui->menuSource_Control->setEnabled(true);
    ui->actionStart->setVisible(true);

    //Set up Git
    git = new GitIntegration(QFileInfo(tslprjPath).path());
    connect(git, SIGNAL(reloadStatusNeeded()), this, SLOT(updateGit()));
    updateGit();

    //Set all text editors to IDE mode
    for (int i = 0; i < ui->tabs->count(); i++) {
        ((TextEditor*) ui->tabs->widget(i))->enableIDEMode();
    }

    //Set File tree root
    projectModel = new QFileSystemModel();
    projectModel->setRootPath(QFileInfo(tslprjPath).path());

    ui->projectTree->setModel(projectModel);
    ui->projectTree->hideColumn(1);
    ui->projectTree->hideColumn(2);
    ui->projectTree->hideColumn(3);
    ui->projectTree->setRootIndex(projectModel->index(QFileInfo(tslprjPath).path()));

    currentProjectFile = tslprjPath;

    QFileSystemWatcher* projectWatcher = new QFileSystemWatcher();
    projectWatcher->addPath(tslprjPath);
    connect(projectWatcher, SIGNAL(fileChanged(QString)), this, SLOT(updateProjectConfiguration()));
}

void MainWindow::on_projectTree_clicked(const QModelIndex &index)
{
    for (int i = 0; i < ui->tabs->count(); i++) {
        if (projectModel->filePath(index) == ((TextEditor*) ui->tabs->widget(i))->filename()) {
            ui->tabs->setCurrentIndex(i);
            return;
        }
    }

    newTab();
    currentDocument()->openFile(projectModel->filePath(index));
}

void MainWindow::updateGit() {
    if (git->needsInit()) {
        ui->sourceControlPanes->setCurrentIndex(1);
    } else {
        ui->sourceControlPanes->setCurrentIndex(0);
        QStringList changedFiles = git->reloadStatus();
        ui->modifiedChanges->clear();

        for (QString changedFile : changedFiles) {
            if (changedFile != "") {
                QChar flag1 = changedFile.at(0);
                QChar flag2 = changedFile.at(1);
                QString fileLocation = changedFile.mid(2);

                QListWidgetItem* item = new QListWidgetItem;
                item->setText(fileLocation);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                if (flag1 == 'A' || flag1 == 'M') {
                    //Staged change
                    item->setCheckState(Qt::Checked);
                } else {
                    //Unstaged change
                    item->setCheckState(Qt::Unchecked);
                }

                ui->modifiedChanges->addItem(item);
            }
        }
    }
}

void MainWindow::on_modifiedChanges_itemChanged(QListWidgetItem *item)
{
    if (item->checkState() == Qt::Checked) {
        git->add(item->text());
    } else {
        git->unstage(item->text());
    }
}

void MainWindow::on_pushButton_clicked()
{
    git->init();
}

void MainWindow::on_actionSave_All_triggered()
{
    for (int i = 0; i < ui->tabs->count(); i++) {
        TextEditor* document = (TextEditor*) ui->tabs->widget(i);
        if (document->filename() != "") {
            document->saveFile();
        }
    }
}

void MainWindow::switchToFile(QString file, QString fakeFileContents) {
    for (int i = 0; i < ui->tabs->count(); i++) {
        if (((TextEditor*) ui->tabs->widget(i))->filename() == file) {
            ui->tabs->setCurrentIndex(i);
            return;
        }
    }

    newTab();
    if (fakeFileContents == "") {
        currentDocument()->openFile(file);
    } else {
        currentDocument()->openFileFake(file, fakeFileContents);
    }
}

void MainWindow::setCurrentDocumentHighlighting(SyntaxHighlighter::codeType type) {
    if (currentDocument() == NULL) {

    } else {
        currentDocument()->highlighter()->setCodeType(type);
    }
}
