#include "authenticationdialog.h"
#include "ui_authenticationdialog.h"

AuthenticationDialog::AuthenticationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthenticationDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->cancelButton->setVisible(false);
    ui->acceptButton->setVisible(false);
#else
    ui->windowControlsMac->setVisible(false);
#endif
}

AuthenticationDialog::~AuthenticationDialog()
{
    delete ui;
}

void AuthenticationDialog::setMessage(QString message) {
    ui->messageLabel->setText(message);
}

QString AuthenticationDialog::username() {
    return ui->usernameBox->text();
}

QString AuthenticationDialog::password() {
    return ui->passwordBox->text();
}
