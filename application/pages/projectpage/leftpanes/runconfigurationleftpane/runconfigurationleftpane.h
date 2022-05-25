#ifndef RUNCONFIGURATIONLEFTPANE_H
#define RUNCONFIGURATIONLEFTPANE_H

#include "../abstractleftpane/abstractleftpane.h"
#include <QUrl>
#include <project.h>

namespace Ui {
    class RunConfigurationLeftPane;
}

struct RunConfigurationLeftPanePrivate;
class RunConfigurationLeftPane : public AbstractLeftPane {
        Q_OBJECT

    public:
        explicit RunConfigurationLeftPane(ProjectPtr project, QWidget* parent = nullptr);
        ~RunConfigurationLeftPane();

    private:
        Ui::RunConfigurationLeftPane* ui;
        RunConfigurationLeftPanePrivate* d;

        void updateRunConfigurations();

        // AbstractLeftPane interface
    public:
        tWindowTabberButton* tabButton();
    private slots:
        void on_runConfigurationsComboBox_activated(int index);
        void on_configureButton_clicked();
        void on_buildButton_clicked();
};

#endif // RUNCONFIGURATIONLEFTPANE_H
