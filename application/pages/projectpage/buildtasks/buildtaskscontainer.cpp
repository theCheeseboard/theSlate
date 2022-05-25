#include "buildtaskscontainer.h"
#include "ui_buildtaskscontainer.h"

#include "buildtaskprogressitem.h"

struct BuildTasksContainerPrivate {
        ProjectPtr project;
        QList<BuildTaskProgressItem*> progressItems;
};

BuildTasksContainer::BuildTasksContainer(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BuildTasksContainer) {
    ui->setupUi(this);
    d = new BuildTasksContainerPrivate;

    updateHeight();
}

BuildTasksContainer::~BuildTasksContainer() {
    delete ui;
    delete d;
}

void BuildTasksContainer::setProject(ProjectPtr project) {
    d->project = project;
    connect(project.data(), &Project::buildJobAdded, this, &BuildTasksContainer::addBuildJob);
}

void BuildTasksContainer::addBuildJob(BuildJobPtr buildJob) {
    auto* item = new BuildTaskProgressItem(buildJob, this);
    d->progressItems.append(item);
    connect(item, &BuildTaskProgressItem::updateHeight, this, &BuildTasksContainer::updateHeight);
    connect(item, &BuildTaskProgressItem::done, this, [=] {
        d->progressItems.removeOne(item);
        ui->jobItems->removeWidget(item);
        item->deleteLater();

        updateHeight();
    });
    ui->jobItems->addWidget(item);

    updateHeight();
}

void BuildTasksContainer::updateHeight() {
    int heightSum = 0;
    for (auto* item : d->progressItems) {
        heightSum += item->sizeHint().height();
    }

    heightSum = qMin(heightSum, 300);
    this->setFixedHeight(heightSum);
}
