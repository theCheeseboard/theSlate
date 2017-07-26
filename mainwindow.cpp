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
    ui->debugFrame->setVisible(false);
    ui->actionFile_in_Project->setVisible(false);
    ui->menuSource_Control->setEnabled(false);
    ui->actionStart->setVisible(false);

    ui->sourceControlOptionsButton->setMenu(ui->menuSource_Control);

    //Set up code highlighting options
    ui->menuCode->addAction("C++", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::cpp);});
    ui->menuCode->addAction("JavaScript", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::js);});
    ui->menuCode->addAction("Python", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::py);});
    ui->menuCode->addAction("XML", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::xml);});
    ui->menuCode->addAction("Markdown", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::md);});
    ui->menuCode->addAction("JavaScript Object Notation (JSON)", [=] {currentDocument()->highlighter()->setCodeType(SyntaxHighlighter::json);});

    addTerminal();

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
            if (currentDocument()->isEdited() || currentDocument()->filename() != "") {
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
    ui->debugFrame->setVisible(true);
    ui->actionFile_in_Project->setVisible(true);
    ui->actionNew_theSlate_Project->setVisible(false);
    ui->menuSource_Control->setEnabled(true);
    ui->actionStart->setVisible(true);

    //Set up Git
    git = new GitIntegration(QFileInfo(tslprjPath).path());
    connect(git, SIGNAL(reloadStatusNeeded()), this, SLOT(updateGit()));
    updateGit();

    //Set File tree root
    projectModel = new QFileSystemModel();
    projectModel->setRootPath(QFileInfo(tslprjPath).path());

    ui->projectTree->setModel(projectModel);
    ui->projectTree->hideColumn(1);
    ui->projectTree->hideColumn(2);
    ui->projectTree->hideColumn(3);
    ui->projectTree->setRootIndex(projectModel->index(QFileInfo(tslprjPath).path()));

    currentProjectFile = tslprjPath;
    updateProjectConfiguration();

    QFileSystemWatcher* projectWatcher = new QFileSystemWatcher();
    projectWatcher->addPath(tslprjPath);
    connect(projectWatcher, SIGNAL(fileChanged(QString)), this, SLOT(updateProjectConfiguration()));
}

TermWidget* MainWindow::addTerminal() {
    #ifdef Q_OS_LINUX
        TerminalWidget* term = new TerminalWidget(QDir::homePath());
        ui->terminals->addWidget(term);
        ui->terminalBox->addItem("Terminal");
        return term;
    #else
        return NULL;
    #endif
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

void MainWindow::on_terminalBox_currentIndexChanged(int index)
{
    ui->terminals->setCurrentIndex(index);
}

void MainWindow::on_terminals_currentChanged(int arg1)
{
    ui->terminalBox->setCurrentIndex(arg1);
}

void MainWindow::on_newTerminalButton_clicked()
{
    addTerminal();
}

void MainWindow::on_closeTerminal_clicked()
{
    int i = ui->terminalBox->currentIndex();
    ui->terminals->widget(i)->deleteLater();
    ui->terminalBox->removeItem(i);
    ui->terminals->removeWidget(ui->terminals->widget(i));
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

void MainWindow::updateProjectConfiguration() {
    ui->runConfigurations->clear();

    QFile configFile(currentProjectFile);
    configFile.open(QFile::ReadOnly);

    QByteArray file = configFile.readAll();
    QJsonParseError* jsonError = new QJsonParseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file, jsonError);
    configFile.close();

    if (!jsonDoc.isNull()) {
        QJsonObject rootObj = jsonDoc.object();
        projectType = rootObj.value("debuggerType").toString();
        QJsonArray configurations = rootObj.value("runTypes").toArray();

        for (QJsonValue configuration : configurations) {
            if (configuration.isObject()) {
                QJsonObject obj = configuration.toObject();
                ui->runConfigurations->addItem(obj.value("name").toString(), obj);
            }
        }
    } else {
        //Configuration error
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

void MainWindow::on_actionStart_triggered()
{
    //Save all files
    ui->actionSave_All->trigger();

    //Run current configuration
    QJsonObject configurationDetails = ui->runConfigurations->itemData(ui->runConfigurations->currentIndex()).toJsonObject();
    QString runFile = configurationDetails.value("program").toString();
    bool debug = configurationDetails.value("debug").toBool();

    TermWidget* term;
    if (debugTerminal == -1) {
        //Create a new terminal for debugging
        term = addTerminal();
        debugTerminal = ui->terminals->indexOf(term);
    } else {
        term = (TermWidget*) ui->terminals->widget(debugTerminal);
    }

    ui->debugTabs->setCurrentIndex(1);
    ui->terminals->setCurrentWidget(term);
    term->changeDir(QFileInfo(currentProjectFile).path());

    if (projectType == "nodejs") {
        if (debug) {
            term->runCommand("node --inspect-brk=47392 " + runFile);

            currentDebugger = new NodeJsDebugger(47392);
            connect(currentDebugger, &Debugger::destroyed, [=] {
                currentDebugger = NULL;
            });

            //Wait 5 seconds and then start debugging
            QTimer::singleShot(1000, [=] {
                currentDebugger->startDebugging();
            });
        } else {
            term->runCommand("node " + runFile);
        }
    }
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
