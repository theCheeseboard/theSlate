#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class AbstractPage;
class RepositoryBrowser;
struct MainWindowPrivate;
class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    private slots:
        void on_actionExit_triggered();

        void on_actionEmpty_Text_File_triggered();

        void on_actionUndo_triggered();

        void on_actionRedo_triggered();

        void on_actionOpenFile_triggered();

        void on_actionSave_triggered();

        void on_actionSave_As_triggered();

        void on_actionSave_All_triggered();

        void on_actionClose_Tab_triggered();

    private:
        Ui::MainWindow* ui;
        MainWindowPrivate* d;

        void addPage(AbstractPage* page);
};

#endif // MAINWINDOW_H
