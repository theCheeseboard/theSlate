#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit SettingsDialog(QWidget *parent = nullptr);
        ~SettingsDialog();
    
    public slots:
        void accept();

    private slots:
        void on_categoryList_currentRowChanged(int currentRow);

        void on_systemMonospaceFont_toggled(bool checked);

        void on_fontBox_currentFontChanged(const QFont &f);

        void on_sizeBox_valueChanged(int arg1);

        void on_showHiddenFiles_toggled(bool checked);

        void on_tabKeySpaces_toggled(bool checked);

        void on_tabKeyTabs_toggled(bool checked);

        void on_tabKeySpaceNunber_valueChanged(int arg1);

        void on_tabCharWidth_valueChanged(int arg1);

        void on_endOfLineComboBox_currentIndexChanged(int index);

        void on_wrapTextBox_toggled(bool checked);

        void on_enableGitSwitch_toggled(bool checked);

        void on_gitPeriodicallyFetch_toggled(bool checked);

        void on_editAStyleConfiguration_clicked();

        void on_formatBeforeSave_toggled(bool checked);

    private:
        Ui::SettingsDialog *ui;

        QSettings settings;
};

#endif // SETTINGSDIALOG_H
