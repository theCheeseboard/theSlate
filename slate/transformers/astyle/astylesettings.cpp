#include "astylesettings.h"
#include "ui_astylesettings.h"

#include <tmessagebox.h>
#include <QMenu>
#include "astyle.h"

struct AStyleSettingsPrivate {
    AStyle as;
};

AStyleSettings::AStyleSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AStyleSettings)
{
    ui->setupUi(this);
    d = new AStyleSettingsPrivate();

    #ifdef Q_OS_MAC
        ui->backButton->setVisible(false);
    #else
        ui->bottomBar->setVisible(false);
    #endif

    loadSettings();
}

AStyleSettings::~AStyleSettings()
{
    delete ui;
    delete d;
}

void AStyleSettings::on_backButton_clicked()
{
    //Save the settings
    d->as.setSettingsContents(ui->settingsFile->toPlainText());
    d->as.saveSettings();
    emit done();
}

void AStyleSettings::loadSettings()
{
    ui->settingsFile->setPlainText(d->as.settingsContents());
}

void AStyleSettings::on_settingsFile_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu(this);

    menu->addSection(tr("Special Values"));
    menu->addAction(tr("Tab Character Length"), [=] {
        ui->settingsFile->textCursor().insertText("%[INDENT_SIZE]");
    });
    menu->addAction(tr("Tab Character"), [=] {
        ui->settingsFile->textCursor().insertText("%[INDENT_TABS]");
    });
    menu->popup(ui->settingsFile->mapToGlobal(pos));
}

void AStyleSettings::on_resetSettings_clicked()
{
    tMessageBox* box = new tMessageBox(this);
    box->setWindowTitle(tr("Reset Artistic Style"));
    box->setText(tr("After resetting Artistic Style settings, theSlate will use the default settings when using Artistic Style to format your code.\n\nYour settings will be lost."));
    box->setIcon(tMessageBox::Warning);
    box->setWindowFlags(Qt::Sheet);
    box->setStandardButtons(tMessageBox::Reset | tMessageBox::Cancel);
    box->setDefaultButton(tMessageBox::Cancel);
    if (box->exec() == tMessageBox::Reset) {
        //Reset Settings
        d->as.resetSettings();
        loadSettings();
    }
    box->deleteLater();
}
