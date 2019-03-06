#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::on_cancelButton_clicked()
{
    emit cancel();
}

void ProgressDialog::setTitle(QString title) {
    ui->title->setText(title);
}

void ProgressDialog::setMessage(QString message) {
    ui->message->setText(message);
}

void ProgressDialog::setCancelable(bool cancelable) {
    ui->cancelButton->setVisible(cancelable);
}
