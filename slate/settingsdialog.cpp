#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    #ifdef Q_OS_MAC
        ui->backButton->setVisible(false);
    #else
        ui->bottomBar->setVisible(false);
    #endif
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_categoryList_currentRowChanged(int currentRow)
{
    ui->categoryStack->setCurrentIndex(currentRow);
}
