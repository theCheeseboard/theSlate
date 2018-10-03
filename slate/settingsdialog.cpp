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

    ui->systemMonospaceFont->setChecked(settings.value("font/useSystem", true).toBool());
    ui->fontBox->setCurrentFont(QFont(settings.value("font/textFontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString()));
    ui->sizeBox->setValue(settings.value("font/textFontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_categoryList_currentRowChanged(int currentRow)
{
    ui->categoryStack->setCurrentIndex(currentRow);
}

void SettingsDialog::on_systemMonospaceFont_toggled(bool checked)
{
    settings.setValue("font/useSystem", checked);
    if (checked) {
        ui->fontBox->setEnabled(false);
        ui->sizeBox->setEnabled(false);
    } else {
        ui->fontBox->setEnabled(true);
        ui->sizeBox->setEnabled(true);
    }
}

void SettingsDialog::on_fontBox_currentFontChanged(const QFont &f)
{
    settings.setValue("font/textFontFamily", f.family());
}

void SettingsDialog::on_sizeBox_valueChanged(int arg1)
{
    settings.setValue("font/textFontSize", arg1);
}
