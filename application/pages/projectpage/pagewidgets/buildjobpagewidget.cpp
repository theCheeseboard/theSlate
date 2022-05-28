#include "buildjobpagewidget.h"
#include "ui_buildjobpagewidget.h"

#include "../models/buildjobissuesmodel.h"
#include <QFontDatabase>
#include <QScrollBar>

struct BuildJobPageWidgetPrivate {
        BuildJobPtr buildJob;
        bool scrollLogToBottom = false;
        BuildJobIssuesModel* issuesModel;
};

BuildJobPageWidget::BuildJobPageWidget(BuildJobPtr buildJob, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BuildJobPageWidget) {
    ui->setupUi(this);
    d = new BuildJobPageWidgetPrivate();
    d->buildJob = buildJob;

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    ui->logBrowser->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(buildJob.data(), &BuildJob::titleChanged, this, [=](QString title) {
        ui->titleLabel->setText(tr("Build log for %1").arg(QLocale().quoteString(title)));
    });
    ui->titleLabel->setText(tr("Build log for %1").arg(QLocale().quoteString(buildJob->title())));

    ui->tabber->addButton(new tWindowTabberButton(QIcon::fromTheme("format-justify-left"), tr("Build Log"), ui->stackedWidget, ui->logPage));
    ui->tabber->addButton(new tWindowTabberButton(QIcon::fromTheme("tools-report-bug"), tr("Issues"), ui->stackedWidget, ui->issuesPage));

    connect(buildJob.data(), &BuildJob::buildLogAppendedTo, this, [=](QString contents) {
        appendToLog(contents);
    });
    appendToLog(buildJob->buildLog());

    connect(ui->logBrowser->verticalScrollBar(), &QScrollBar::valueChanged, this, [=](int value) {
        d->scrollLogToBottom = ui->logBrowser->verticalScrollBar()->maximum() == value;
    });
    connect(ui->logBrowser->verticalScrollBar(), &QScrollBar::rangeChanged, this, [=](int min, int max) {
        if (d->scrollLogToBottom) ui->logBrowser->verticalScrollBar()->setValue(max);
    });
    ui->logBrowser->verticalScrollBar()->setValue(ui->logBrowser->verticalScrollBar()->maximum());
    d->scrollLogToBottom = true;

    d->issuesModel = new BuildJobIssuesModel(d->buildJob);
    ui->issuesList->setModel(d->issuesModel);
    ui->issuesList->setItemDelegate(new BuildJobIssuesDelegate(this));
}

BuildJobPageWidget::~BuildJobPageWidget() {
    delete ui;
    delete d;
}

void BuildJobPageWidget::appendToLog(QString contents) {
    ui->logBrowser->append(contents);
}

void BuildJobPageWidget::on_issuesList_clicked(const QModelIndex& index) {
    emit requestFileOpen(index.data(BuildJobIssuesModel::EditorUrl).toUrl());
}
