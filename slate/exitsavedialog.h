#ifndef EXITSAVEDIALOG_H
#define EXITSAVEDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "textwidget.h"
#include <QFileDialog>

namespace Ui {
class ExitSaveDialog;
}

class ExitSaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExitSaveDialog(QList<TextWidget*> saveNeeded, QWidget *parent = 0);
    ~ExitSaveDialog();

private slots:
    void on_editedFilesList_itemChanged(QListWidgetItem *item);

    void on_cancelButton_clicked();

    void on_saveButton_clicked();

    void on_discardButton_clicked();

signals:
    void closeTab(TextWidget* tab);

private:
    Ui::ExitSaveDialog *ui;
    QList<TextWidget*> saveNeeded;
    bool attnAll = true;
};

#endif // EXITSAVEDIALOG_H
