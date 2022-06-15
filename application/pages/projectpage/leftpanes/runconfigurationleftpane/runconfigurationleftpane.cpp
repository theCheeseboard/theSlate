#include "runconfigurationleftpane.h"
#include "ui_runconfigurationleftpane.h"

#include "../../pagewidgets/buildjobpagewidget.h"
#include "runconfigurationbuilditem.h"
#include "widgetholder/widgetholdereditor.h"
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

    ui->buildsLayout->setDirection(QBoxLayout::BottomToTop);

    connect(project.data(), &Project::runConfigurationsUpdated, this, &RunConfigurationLeftPane::updateRunConfigurations);
    connect(project.data(), &Project::targetsChanged, this, &RunConfigurationLeftPane::updateTargets);
    connect(project.data(), &Project::currentTargetChanged, this, &RunConfigurationLeftPane::updateTargets);
    updateRunConfigurations();
    updateTargets();

    connect(project.data(), &Project::buildJobAdded, this, [=](BuildJobPtr job) {
        auto* item = new RunConfigurationBuildItem(job);
        connect(item, &RunConfigurationBuildItem::requestFileOpen, this, &RunConfigurationLeftPane::requestFileOpen);
        ui->buildsLayout->addWidget(item);
    });
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

void RunConfigurationLeftPane::updateTargets() {
    QSignalBlocker blocker(ui->targetsBox);
    ui->targetsBox->clear();

    auto targets = d->project->targets();
    ui->targetsBox->addItems(targets);
    ui->targetsBox->setCurrentIndex(targets.indexOf(d->project->activeTarget()));
    ui->runButton->setEnabled(d->project->canActiveRunConfigurationRun());
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

void RunConfigurationLeftPane::on_targetsBox_currentIndexChanged(int index) {
    d->project->setActiveTarget(ui->targetsBox->itemText(index));
}

void RunConfigurationLeftPane::on_runButton_clicked() {
    d->project->activeRunConfigurationRun();
}
