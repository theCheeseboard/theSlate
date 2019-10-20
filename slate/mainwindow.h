#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
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
#include "textparts/texteditor.h"
#include "SourceControl/gitintegration.h"
#include "textparts/printdialog.h"

#include <Definition>

#ifdef Q_OS_MAC
#include <QMacToolBar>
#include <QMacToolBarItem>
#endif

class FileBackend;

namespace Ui {
    class MainWindow;
}

class TextWidget;
struct MainWindowPrivate;
class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        static QList<MainWindow*> openWindows;

    public slots:
        TextWidget* newTab();
        void newTab(QString filename);
        void newTab(QByteArray contents);
        void newTab(FileBackend* backend);
        bool closeCurrentTab();
        void show();

        void updateGit();

        TextWidget* currentDocument();
        TextEditor* currentEditor();

        QVariant getOpenOption(QString option);

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

        void on_projectTree_clicked(const QModelIndex &index);

        void on_actionSave_All_triggered();

        void on_actionFind_and_Replace_triggered();

        void on_actionSave_As_triggered();

        void on_actionRevert_triggered();

        void on_actionPrint_triggered();

        void on_actionFile_Bug_triggered();

        void on_actionSources_triggered();

        void on_actionUndo_triggered();

        void on_actionRedo_triggered();

        void on_actionSettings_triggered();

        void on_actionClose_triggered();

        void on_actionNew_Window_triggered();

        void on_projectTree_customContextMenuRequested(const QPoint &pos);

        void on_actionSelect_All_triggered();

        void updateRecentFiles();

        void on_actionUse_Menubar_toggled(bool arg1);

        void on_actionComment_triggered();

        void on_actionChange_Syntax_Highlighting_triggered();

        void on_actionChange_File_Encoding_triggered();

        void on_actionReload_Using_Encoding_triggered();

        void on_actionLine_triggered();

        void on_actionUppercase_triggered();

        void on_actionLowercase_triggered();

        void on_actionTitle_Case_triggered();

        void on_actionBeautify_triggered();

    signals:
#ifdef Q_OS_MAC
        void changeTouchBarTopNotification(TopNotification* topNotification);
#endif

private:
        Ui::MainWindow *ui;
        MainWindowPrivate* d;

        void closeEvent(QCloseEvent* event);

        void dragEnterEvent(QDragEnterEvent* event);
        void dragLeaveEvent(QDragLeaveEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);
        void changeEvent(QEvent* event);

        void updateDocumentDependantTabs();

#ifdef Q_OS_MAC
        void setupMacOS();
        void updateTouchBar();
#endif
};

#endif // MAINWINDOW_H
