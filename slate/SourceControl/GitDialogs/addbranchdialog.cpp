#include "addbranchdialog.h"
#include "ui_addbranchdialog.h"

#include "../branchesmodel.h"
#include <tmessagebox.h>

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

void AddBranchDialog::on_acceptButton_clicked()
{
    for (int i = 0; i < ui->branchFromBox->count(); i++) {
        GitIntegration::BranchPointer branch = ui->branchFromBox->itemData(i, Qt::UserRole + 1).value<GitIntegration::BranchPointer>();
        if (branch->name == ui->branchName->text()) {
            //We've got a duplicate branch folks!
            tMessageBox* messageBox = new tMessageBox(this);
            messageBox->setWindowTitle(tr("Can't branch"));
            messageBox->setText(tr("The branch %1 already exists.").arg(ui->branchName->text()));
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->exec();
            messageBox->deleteLater();
            return;
        }
    }

    this->accept();
}
