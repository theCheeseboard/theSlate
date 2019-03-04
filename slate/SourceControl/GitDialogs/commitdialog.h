#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

#include <QDialog>
#include "../gitintegration.h"

namespace Ui {
class CommitDialog;
}

struct CommitDialogPrivate;
class MainWindow;
class CommitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommitDialog(GitIntegration* integration, MainWindow* mainWin, QWidget *parent = nullptr);
    ~CommitDialog();

    private slots:
        void on_acceptButton_clicked();

    private:
    Ui::CommitDialog *ui;

    CommitDialogPrivate* d;
};

#endif // COMMITDIALOG_H
