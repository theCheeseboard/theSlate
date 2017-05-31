#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include "aboutwindow.h"
#include "texteditor.h"

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

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent* event);
};

#endif // MAINWINDOW_H
