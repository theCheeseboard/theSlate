#ifndef ADDBRANCHDIALOG_H
#define ADDBRANCHDIALOG_H

#include <QDialog>
#include "../gitintegration.h"

namespace Ui {
class AddBranchDialog;
}

class AddBranchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBranchDialog(GitIntegration* integration, QWidget *parent = nullptr);
    ~AddBranchDialog();

    GitIntegration::BranchPointer from();
    QString name();

private:
    Ui::AddBranchDialog *ui;
};

#endif // ADDBRANCHDIALOG_H
