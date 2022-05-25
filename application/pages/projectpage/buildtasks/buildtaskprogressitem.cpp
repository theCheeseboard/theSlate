#include "buildtaskprogressitem.h"
#include "ui_buildtaskprogressitem.h"

#include <QPainter>
#include <QTimer>

struct BuildTaskProgressItemPrivate {
        BuildJobPtr buildJob;
};

BuildTaskProgressItem::BuildTaskProgressItem(BuildJobPtr buildJob, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BuildTaskProgressItem) {
    ui->setupUi(this);
    d = new BuildTaskProgressItemPrivate();
    d->buildJob = buildJob;

    connect(buildJob.data(), &BuildJob::titleChanged, this, &BuildTaskProgressItem::updateTitle);
    connect(buildJob.data(), &BuildJob::descriptionChanged, this, &BuildTaskProgressItem::updateDescription);
    connect(buildJob.data(), &BuildJob::progressChanged, this, &BuildTaskProgressItem::updateProgress);
    connect(buildJob.data(), &BuildJob::stepChanged, this, &BuildTaskProgressItem::updateSteps);
    connect(buildJob.data(), &BuildJob::stateChanged, this, &BuildTaskProgressItem::stateChanged);

    updateTitle(buildJob->title());
    updateDescription(buildJob->description());
    updateProgress(buildJob->progress(), buildJob->maxProgress());
    updateSteps(buildJob->step(), buildJob->maxStep());
}

BuildTaskProgressItem::~BuildTaskProgressItem() {
    delete ui;
    delete d;
}

void BuildTaskProgressItem::updateTitle(QString title) {
    ui->titleLabel->setText(title.toUpper());
}

void BuildTaskProgressItem::updateDescription(QString description) {
    ui->descriptionLabel->setText(description);
}

void BuildTaskProgressItem::updateProgress(int progress, int maxProgress) {
    this->update();
}

void BuildTaskProgressItem::updateSteps(int steps, int maxSteps) {
    if (maxSteps == 0) {
        ui->stepsLabel->setVisible(false);
    } else {
        ui->stepsLabel->setText(QStringLiteral("%1 / %2").arg(steps).arg(maxSteps));
        ui->stepsLabel->setVisible(true);
    }
}

void BuildTaskProgressItem::stateChanged(BuildJob::State state) {
    if (state == BuildJob::Successful) {
        QTimer::singleShot(5000, this, &BuildTaskProgressItem::animateOut);
    } else if (state == BuildJob::Failed) {
        QTimer::singleShot(5000, this, &BuildTaskProgressItem::animateOut);
    }
    this->update();
}

void BuildTaskProgressItem::animateOut() {
    emit done();
}

void BuildTaskProgressItem::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    switch (d->buildJob->state()) {
        case BuildJob::Running:
            if (d->buildJob->maxProgress() == 0) {
                // TODO: Indeterminate bar
            } else {
                painter.fillRect(QRect(0, 0, this->width() * (d->buildJob->progress() / static_cast<qreal>(d->buildJob->maxProgress())), this->height()), QColor(255, 255, 255, 100));
            }
            break;
        case BuildJob::Failed:
            painter.fillRect(QRect(0, 0, this->width(), this->height()), QColor(160, 50, 50, 100));
            break;
        case BuildJob::Successful:
            painter.fillRect(QRect(0, 0, this->width(), this->height()), QColor(50, 160, 80, 100));
            break;
    }
}
