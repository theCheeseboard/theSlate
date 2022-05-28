#include "runconfigurationbuilditem.h"
#include "ui_runconfigurationbuilditem.h"

#include "../../pagewidgets/buildjobpagewidget.h"
#include "widgetholder/widgetholdereditor.h"
#include <QDateTime>
#include <QIcon>
#include <QMouseEvent>
#include <libcontemporary_global.h>

struct RunConfigurationBuildItemPrivate {
        BuildJobPtr trackedBuildJob;
};

RunConfigurationBuildItem::RunConfigurationBuildItem(BuildJobPtr buildJob, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::RunConfigurationBuildItem) {
    ui->setupUi(this);
    d = new RunConfigurationBuildItemPrivate();
    d->trackedBuildJob = buildJob;

    ui->timestampLabel->setText(QLocale().toString(buildJob->buildStartDate().time(), QLocale::ShortFormat));

    connect(buildJob.data(), &BuildJob::titleChanged, this, &RunConfigurationBuildItem::setTitle);
    connect(buildJob.data(), &BuildJob::stateChanged, this, &RunConfigurationBuildItem::setState);
    this->setTitle(buildJob->title());
    this->setState(buildJob->state());
}

RunConfigurationBuildItem::~RunConfigurationBuildItem() {
    delete ui;
    delete d;
}

void RunConfigurationBuildItem::setTitle(QString title) {
    ui->titleLabel->setText(title);
}

void RunConfigurationBuildItem::setState(BuildJob::State state) {
    switch (state) {
        case BuildJob::Running:
            ui->iconLabel->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(SC_DPI_WT(QSize(16, 16), QSize, this)));
            break;
        case BuildJob::Failed:
            ui->iconLabel->setPixmap(QIcon::fromTheme("emblem-warning").pixmap(SC_DPI_WT(QSize(16, 16), QSize, this)));
            break;
        case BuildJob::Successful:
            ui->iconLabel->setPixmap(QIcon::fromTheme("emblem-checked").pixmap(SC_DPI_WT(QSize(16, 16), QSize, this)));
            break;
    }
}

void RunConfigurationBuildItem::mousePressEvent(QMouseEvent* event) {
    event->accept();
}

void RunConfigurationBuildItem::mouseReleaseEvent(QMouseEvent* event) {
    if (this->underMouse() && event->button() == Qt::LeftButton && event->buttons() == Qt::NoButton) {
        auto* pageWidget = new BuildJobPageWidget(d->trackedBuildJob);
        connect(pageWidget, &BuildJobPageWidget::requestFileOpen, this, &RunConfigurationBuildItem::requestFileOpen);
        emit requestFileOpen(WidgetHolderEditor::urlForWidget(pageWidget));
    }
}

void RunConfigurationBuildItem::enterEvent(QEnterEvent* event) {
}

void RunConfigurationBuildItem::leaveEvent(QEvent* event) {
}
