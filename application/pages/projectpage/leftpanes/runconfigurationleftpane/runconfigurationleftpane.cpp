#include "runconfigurationleftpane.h"
#include "ui_runconfigurationleftpane.h"

#include <twindowtabberbutton.h>

struct RunConfigurationLeftPanePrivate {
        tWindowTabberButton* tabButton;
        ProjectPtr project;
};

RunConfigurationLeftPane::RunConfigurationLeftPane(ProjectPtr project, QWidget* parent) :
    AbstractLeftPane(parent),
    ui(new Ui::RunConfigurationLeftPane) {
    ui->setupUi(this);
    d = new RunConfigurationLeftPanePrivate();
    d->project = project;

    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText("Run");

    connect(project.data(), &Project::runConfigurationsUpdated, this, &RunConfigurationLeftPane::updateRunConfigurations);
    updateRunConfigurations();
}

RunConfigurationLeftPane::~RunConfigurationLeftPane() {
    delete ui;
    delete d;
}

void RunConfigurationLeftPane::updateRunConfigurations() {
    ui->runConfigurationsComboBox->clear();

    QSignalBlocker blocker(ui->runConfigurationsComboBox);
    if (d->project->runConfigurations().isEmpty()) {
        ui->runConfigurationsComboBox->setEnabled(false);
    } else {
        for (auto runConfiguration : d->project->runConfigurations()) {
            ui->runConfigurationsComboBox->addItem(runConfiguration->name(), QVariant::fromValue(runConfiguration));
            if (d->project->activeRunConfiguration() == runConfiguration) ui->runConfigurationsComboBox->setCurrentIndex(ui->runConfigurationsComboBox->count() - 1);
        }
        ui->runConfigurationsComboBox->setEnabled(true);
    }

    ui->configureButton->setVisible(d->project->canActiveRunConfigurationConfigure());
    ui->buildButton->setVisible(d->project->canActiveRunConfigurationBuild());
}

tWindowTabberButton* RunConfigurationLeftPane::tabButton() {
    return d->tabButton;
}

void RunConfigurationLeftPane::on_runConfigurationsComboBox_activated(int index) {
    auto selectedConfiguration = ui->runConfigurationsComboBox->currentData().value<RunConfigurationPtr>();
    if (selectedConfiguration) d->project->setActiveRunConfiguration(selectedConfiguration);
}

void RunConfigurationLeftPane::on_configureButton_clicked() {
    d->project->activeRunConfigurationConfigure();
}

void RunConfigurationLeftPane::on_buildButton_clicked() {
    d->project->activeRunConfigurationBuild();
}
