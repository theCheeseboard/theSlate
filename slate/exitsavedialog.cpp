#include "exitsavedialog.h"
#include "ui_exitsavedialog.h"

#include <QFileDialog>
#include <QEventLoop>

ExitSaveDialog::ExitSaveDialog(QList<TextEditor*> saveNeeded, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExitSaveDialog)
{
    ui->setupUi(this);

    this->saveNeeded = saveNeeded;
    for (TextEditor* editor : saveNeeded) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(editor->getTabButton()->text());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant::fromValue(editor));
        ui->editedFilesList->addItem(item);
    }
}

ExitSaveDialog::~ExitSaveDialog()
{
    delete ui;
}

void ExitSaveDialog::on_editedFilesList_itemChanged(QListWidgetItem *item)
{
    int numberChecked = 0;
    for (int i = 0; i < ui->editedFilesList->count(); i++) {
        if (ui->editedFilesList->item(i)->checkState() == Qt::Checked) numberChecked++;
    }

    if (numberChecked == 0 || numberChecked == ui->editedFilesList->count()) {
        ui->saveButton->setText(tr("Save All"));
        ui->discardButton->setText(tr("Discard All"));
        attnAll = true;
    } else {
        ui->saveButton->setText(tr("Save %n", nullptr, numberChecked));
        ui->discardButton->setText(tr("Discard %n", nullptr, numberChecked));
        attnAll = false;
    }
}

void ExitSaveDialog::on_cancelButton_clicked()
{
    this->close();
}

void ExitSaveDialog::on_saveButton_clicked()
{
    bool didHide = false;
    for (int i = 0; i < ui->editedFilesList->count(); i++) {
        if (ui->editedFilesList->item(i)->checkState() == Qt::Checked || attnAll) {
            TextEditor* editor = ui->editedFilesList->item(i)->data(Qt::UserRole).value<TextEditor*>();
            if (editor->title() == "" && !didHide) {
                this->hide();
                didHide = true;
            }

            if (editor->saveFileAskForFilename()) {
                delete ui->editedFilesList->takeItem(i);
                emit closeTab(editor);
                i--;
            }
        }
    }

    if (ui->editedFilesList->count() == 0) {
        this->close();
        emit closeWindow();
    } else if (didHide) {
        this->show();
        on_editedFilesList_itemChanged(ui->editedFilesList->item(0));
    } else {
        on_editedFilesList_itemChanged(ui->editedFilesList->item(0));
    }
}

void ExitSaveDialog::on_discardButton_clicked()
{
    for (int i = 0; i < ui->editedFilesList->count(); i++) {
        if (ui->editedFilesList->item(i)->checkState() == Qt::Checked || attnAll) {
            TextEditor* editor = ui->editedFilesList->item(i)->data(Qt::UserRole).value<TextEditor*>();
            delete ui->editedFilesList->takeItem(i);
            emit closeTab(editor);
            i--;
        }
    }

    if (ui->editedFilesList->count() == 0) {
        this->close();
        emit closeWindow();
    } else {
        on_editedFilesList_itemChanged(ui->editedFilesList->item(0));
    }
}
