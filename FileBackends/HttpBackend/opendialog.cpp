#include "opendialog.h"
#include "ui_opendialog.h"

OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
    ui->urlError->setVisible(false);
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::on_openButton_clicked()
{
    if (cUrl.isValid()) {
        this->accept();
    }
}

void OpenDialog::on_cancelButton_clicked()
{
    this->reject();
}

QUrl OpenDialog::currentUrl() {
    return cUrl;
}

void OpenDialog::on_urlToOpen_textChanged(const QString &arg1)
{
    cUrl = QUrl::fromUserInput(arg1);
}

bool OpenDialog::redirect() {
    return ui->redirectCheckbox->isChecked();
}

void OpenDialog::on_urlToOpen_editingFinished()
{
    if (cUrl.isValid()) {
        ui->urlError->setVisible(false);
    } else {
        ui->urlError->setVisible(true);
    }
}
