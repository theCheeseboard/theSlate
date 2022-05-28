#include "buildjob.h"

#include "logscannermanager.h"
#include "statemanager.h"
#include <QDateTime>

struct BuildJobPrivate {
        BuildJob::State state = BuildJob::Running;

        QString buildLog;
        QList<BuildJob::BuildIssue> buildIssues;

        QString title;
        QString description;

        int progress = 0;
        int maxProgress = 0;
        int step = 0;
        int maxStep = 0;

        QDateTime buildStartDate;
};

BuildJob::BuildJob(QObject* parent) :
    QObject{parent} {
    d = new BuildJobPrivate();
    d->buildStartDate = QDateTime::currentDateTime();
}

BuildJob::~BuildJob() {
    delete d;
}

BuildJob::State BuildJob::state() {
    return d->state;
}

QString BuildJob::title() {
    return d->title;
}

QString BuildJob::description() {
    return d->description;
}

int BuildJob::progress() {
    return d->progress;
}

int BuildJob::maxProgress() {
    return d->maxProgress;
}

int BuildJob::step() {
    return d->step;
}

int BuildJob::maxStep() {
    return d->maxStep;
}

QString BuildJob::buildLog() {
    return d->buildLog;
}

QList<BuildJob::BuildIssue> BuildJob::buildIssues() {
    return d->buildIssues;
}

int BuildJob::countIssues(BuildIssue::Type issueType) {
    int sum = 0;
    for (const auto& issue : d->buildIssues) {
        if (issue.issueType == issueType) sum++;
    }
    return sum;
}

QDateTime BuildJob::buildStartDate() {
    return d->buildStartDate;
}

void BuildJob::setState(State state) {
    d->state = state;
    emit stateChanged(state);
}

void BuildJob::setTitle(QString title) {
    d->title = title;
    emit titleChanged(title);
}

void BuildJob::setDescription(QString description) {
    d->description = description;
    emit descriptionChanged(description);
}

void BuildJob::setProgress(int progress) {
    d->progress = progress;
    emit progressChanged(progress, d->maxProgress);
}

void BuildJob::setMaxProgress(int maxProgress) {
    d->maxProgress = maxProgress;
    emit progressChanged(d->progress, maxProgress);
}

void BuildJob::setStep(int step) {
    d->step = step;
    emit stepChanged(step, d->maxStep);
}

void BuildJob::setMaxStep(int maxStep) {
    d->maxStep = maxStep;
    emit stepChanged(d->step, maxStep);
}

void BuildJob::appendToBuildLog(QString buildLog) {
    d->buildLog.append(buildLog);
    emit buildLogAppendedTo(buildLog);

    for (const auto& line : buildLog.split("\n")) {
        auto issues = StateManager::logScanner()->scanLine(line);
        d->buildIssues.append(issues);
        for (const auto& issue : issues) {
            emit buildIssuesAppendedTo(issue);
        }
    }
}

void BuildJob::appendToBuildIssues(BuildIssue issue) {
    d->buildIssues.append(issue);
    emit buildIssuesAppendedTo(issue);
}
