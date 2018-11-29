#include "httpbasicauthdialog.h"
#include "ui_httpbasicauthdialog.h"

HttpBasicAuthDialog::HttpBasicAuthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HttpBasicAuthDialog)
{
    ui->setupUi(this);
}

HttpBasicAuthDialog::~HttpBasicAuthDialog()
{
    delete ui;
}

void HttpBasicAuthDialog::on_okButton_clicked()
{
    this->accept();
}

void HttpBasicAuthDialog::on_cancelButton_clicked()
{
    this->reject();
}

QString HttpBasicAuthDialog::username() {
    return ui->usernameBox->text();
}

QString HttpBasicAuthDialog::password() {
    return ui->passwordBox->text();
}

void HttpBasicAuthDialog::setText(QString text) {
    ui->realmLabel->setText(text);
}
