#ifndef ASTYLESETTINGS_H
#define ASTYLESETTINGS_H

#include <QWidget>

namespace Ui {
    class AStyleSettings;
}

struct AStyleSettingsPrivate;
class AStyleSettings : public QWidget
{
        Q_OBJECT

    public:
        explicit AStyleSettings(QWidget *parent = nullptr);
        ~AStyleSettings();

    private slots:
        void on_backButton_clicked();

        void on_settingsFile_customContextMenuRequested(const QPoint &pos);

        void on_resetSettings_clicked();

    signals:
        void done();

    private:
        Ui::AStyleSettings *ui;
        AStyleSettingsPrivate* d;

        void loadSettings();
};

#endif // AStyleSETTINGS_H
