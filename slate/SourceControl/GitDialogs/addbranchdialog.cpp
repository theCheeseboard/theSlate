#include "addbranchdialog.h"
#include "ui_addbranchdialog.h"

#include "../branchesmodel.h"

AddBranchDialog::AddBranchDialog(GitIntegration* integration, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddBranchDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->cancelButton->setVisible(false);
    ui->acceptButton->setVisible(false);
#else
    ui->windowControlsMac->setVisible(false);
#endif

    ui->branchFromBox->setModel(new BranchesModel(integration, false));
}

AddBranchDialog::~AddBranchDialog()
{
    delete ui;
}

GitIntegration::BranchPointer AddBranchDialog::from() {
    return ui->branchFromBox->currentData(Qt::UserRole + 1).value<GitIntegration::BranchPointer>();
}

QString AddBranchDialog::name() {
    return ui->branchName->text();
}
