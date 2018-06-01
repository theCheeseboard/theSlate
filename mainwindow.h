#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileSystemModel>
#include <QTreeView>
#include <QStackedWidget>
#include <QAction>
#include <QComboBox>
#include <QLayout>
#include <QListWidget>
#include <QDebug>
#include <QMenu>
#include <QTimer>
#include <QToolBar>
#include <QSettings>
#include <QDockWidget>
#include "aboutwindow.h"
#include "texteditor.h"
#include "SourceControl/gitintegration.h"

#ifdef Q_OS_MAC
#include <QMacToolBar>
#include <QMacToolBarItem>
#endif

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void newTab();
    bool closeCurrentTab();

    TextEditor* currentDocument();

private slots:
    void on_actionNew_triggered();

    void on_tabs_currentChanged(int arg1);

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void checkForEdits();

    bool saveCurrentDocument();

    void on_closeButton_clicked();

    void on_actionCopy_triggered();

    void on_actionCut_triggered();

    void on_actionPaste_triggered();

    void on_actionAbout_triggered();

    void on_actionNo_Highlighting_triggered();

    void on_projectTree_clicked(const QModelIndex &index);

    void updateGit();

    void on_modifiedChanges_itemChanged(QListWidgetItem *item);

    void on_actionSave_All_triggered();

    void switchToFile(QString file, QString fakeFileContents = "");

    void setCurrentDocumentHighlighting(SyntaxHighlighter::codeType type);

    void on_initGitButton_clicked();

    void on_actionShowSourceControlWindow_triggered();

    void on_sourceControlDock_visibilityChanged(bool visible);

    private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent* event);
    QFileSystemModel* fileModel;
    QString currentProjectFile = "";
    QString projectType = "";
    QSettings settings;
};

#endif // MAINWINDOW_H
