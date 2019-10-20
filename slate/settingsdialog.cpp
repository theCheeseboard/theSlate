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

    ui->mainStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->leftPane->setFixedWidth(SC_DPI(300));

    ui->systemMonospaceFont->setChecked(settings.value("font/useSystem", true).toBool());
    ui->fontBox->setCurrentFont(QFont(settings.value("font/textFontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString()));
    ui->sizeBox->setValue(settings.value("font/textFontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());
    ui->showHiddenFiles->setChecked(settings.value("files/showHidden", false).toBool());

    if (settings.value("behaviour/tabSpaces", true).toBool()) {
        ui->tabKeySpaces->setChecked(true);
    } else {
        ui->tabKeyTabs->setChecked(true);
    }
    ui->tabKeySpaceNunber->setValue(settings.value("behaviour/tabSpaceNumber", 4).toInt());
    ui->tabCharWidth->setValue(settings.value("behaviour/tabWidth", 4).toInt());
    ui->wrapTextBox->setChecked(settings.value("behaviour/wrapText", false).toBool());

    ui->endOfLineComboBox->setCurrentIndex(settings.value("behaviour/endOfLine", THESLATE_END_OF_LINE).toInt());

    ui->enableGitSwitch->setChecked(settings.value("git/enable", true).toBool());
    ui->gitPeriodicallyFetch->setChecked(settings.value("git/periodicallyFetch", true).toBool());

    connect(ui->astyleSettings, &AStyleSettings::done, ui->mainStack, std::bind(&tStackedWidget::setCurrentWidget, ui->mainStack, ui->mainPage, true));
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

void SettingsDialog::on_showHiddenFiles_toggled(bool checked)
{
    settings.setValue("files/showHidden", checked);
}

void SettingsDialog::on_tabKeySpaces_toggled(bool checked)
{
    ui->tabKeySpaceNunber->setEnabled(checked);
    if (checked) {
        settings.setValue("behaviour/tabSpaces", true);
    }
}

void SettingsDialog::on_tabKeyTabs_toggled(bool checked)
{
    if (checked) {
        settings.setValue("behaviour/tabSpaces", false);
    }
}

void SettingsDialog::on_tabKeySpaceNunber_valueChanged(int arg1)
{
    settings.setValue("behaviour/tabSpaceNumber", arg1);
}

void SettingsDialog::on_tabCharWidth_valueChanged(int arg1)
{
    settings.setValue("behaviour/tabWidth", arg1);
}

void SettingsDialog::on_endOfLineComboBox_currentIndexChanged(int index)
{
    settings.setValue("behaviour/endOfLine", index);
}

void SettingsDialog::accept() {
    settings.sync();
    QDialog::accept();
}

void SettingsDialog::on_wrapTextBox_toggled(bool checked)
{
    settings.setValue("behaviour/wrapText", checked);
}

void SettingsDialog::on_enableGitSwitch_toggled(bool checked)
{
    settings.setValue("git/enable", checked);
}

void SettingsDialog::on_gitPeriodicallyFetch_toggled(bool checked)
{
    settings.setValue("git/periodicallyFetch", checked);
}

void SettingsDialog::on_editAStyleConfiguration_clicked()
{
    ui->mainStack->setCurrentWidget(ui->astyleSettings);
}
