#include "aboutwindow.h"
#include "ui_aboutwindow.h"

#include "the-libs_global.h"
#include "managers/updatemanager.h"

extern UpdateManager* updateManager;

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    this->resize(this->size() * theLibsGlobal::getDPIScaling());
    ui->versionLabel->setText(tr("theSlate %1").arg(updateManager->versionString()));
}

AboutWindow::~AboutWindow()
{
    delete ui;
}

void AboutWindow::on_okButton_clicked()
{
    this->close();
}
