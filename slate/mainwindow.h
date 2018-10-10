#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
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
#include <QTabBar>
#include "aboutwindow.h"
#include "texteditor.h"
#include "SourceControl/gitintegration.h"
#include "plugins/syntaxhighlighting.h"
#include "textparts/printdialog.h"
#include "plugins/filebackend.h"

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

        static QList<MainWindow*> openWindows;

    public slots:
        void newTab();
        void newTab(QString filename);
        void newTab(FileBackend* backend);
        bool closeCurrentTab();
        void show();

        TextEditor* currentDocument();

    private slots:
        void on_actionNew_triggered();

        void on_tabs_currentChanged(int arg1);

        void on_actionExit_triggered();

        void on_actionOpen_triggered();

        void on_actionSave_triggered();

        void checkForEdits();

        bool saveCurrentDocument(bool saveAs = false);

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

        void setCurrentDocumentHighlighting(QSyntaxHighlighter* highlighter);

        void on_initGitButton_clicked();

        void on_actionFind_and_Replace_triggered();

        void on_actionSave_As_triggered();

        void on_actionRevert_triggered();

        void on_gitAbortMergeButton_clicked();

        void on_actionPull_triggered();

        void on_actionPush_triggered();

        void on_commitButton_clicked();


        void on_actionPrint_triggered();

        void on_actionFile_Bug_triggered();

        void on_actionSources_triggered();

        void on_actionUndo_triggered();

        void on_actionRedo_triggered();

        void on_actionSettings_triggered();

        void on_actionClose_triggered();

        void on_actionNew_Window_triggered();

        void on_projectTree_customContextMenuRequested(const QPoint &pos);

signals:
#ifdef Q_OS_MAC
        void changeTouchBarTopNotification(TopNotification* topNotification);
#endif

private:
        Ui::MainWindow *ui;

        void closeEvent(QCloseEvent* event);

        void dragEnterEvent(QDragEnterEvent* event);
        void dragLeaveEvent(QDragLeaveEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);

#ifdef Q_OS_MAC
        void setupMacOS();
        void updateTouchBar();
#endif

        QFileSystemModel* fileModel;
        QString currentProjectFile = "";
        QString projectType = "";
        QSettings settings;
        QTabBar* tabBar;
        QMap<TextEditor*, TopNotification*> primaryTopNotifications;
};

#endif // MAINWINDOW_H
