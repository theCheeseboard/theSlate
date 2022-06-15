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

    signals:
        void requestFileOpen(QUrl url);

    private:
        Ui::RunConfigurationLeftPane* ui;
        RunConfigurationLeftPanePrivate* d;

        void updateRunConfigurations();
        void updateTargets();

        // AbstractLeftPane interface
    public:
        tWindowTabberButton* tabButton();
    private slots:
        void on_runConfigurationsComboBox_activated(int index);
        void on_configureButton_clicked();
        void on_buildButton_clicked();
        void on_targetsBox_currentIndexChanged(int index);
        void on_runButton_clicked();
};

#endif // RUNCONFIGURATIONLEFTPANE_H
