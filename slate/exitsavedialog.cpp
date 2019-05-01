#include "exitsavedialog.h"
#include "ui_exitsavedialog.h"

#include <QFileDialog>
#include <QEventLoop>
#include "the-libs_global.h"
#include "textparts/texteditor.h"

ExitSaveDialog::ExitSaveDialog(QList<TextWidget*> saveNeeded, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExitSaveDialog)
{
    ui->setupUi(this);
    this->resize(this->size() * theLibsGlobal::getDPIScaling());

    this->saveNeeded = saveNeeded;
    for (TextWidget* editor : saveNeeded) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(editor->editor()->getTabButton()->text());
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
    this->reject();
}

void ExitSaveDialog::on_saveButton_clicked()
{
    bool didHide = false;
    for (int i = 0; i < ui->editedFilesList->count(); i++) {
        if (ui->editedFilesList->item(i)->checkState() == Qt::Checked || attnAll) {
            TextWidget* editor = ui->editedFilesList->item(i)->data(Qt::UserRole).value<TextWidget*>();
            if (editor->editor()->title() == "" && !didHide) {
                this->hide();
                didHide = true;
            }

            if (editor->editor()->saveFileAskForFilename()) {
                delete ui->editedFilesList->takeItem(i);
                emit closeTab(editor);
                i--;
            }
        }
    }

    if (ui->editedFilesList->count() == 0) {
        this->accept();
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
            TextWidget* editor = ui->editedFilesList->item(i)->data(Qt::UserRole).value<TextWidget*>();
            delete ui->editedFilesList->takeItem(i);
            emit closeTab(editor);
            i--;
        }
    }

    if (ui->editedFilesList->count() == 0) {
        this->accept();
    } else {
        on_editedFilesList_itemChanged(ui->editedFilesList->item(0));
    }
}
