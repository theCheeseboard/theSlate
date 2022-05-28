#include "clanglogscanner.h"

#include <QRegularExpression>

struct ClangLogScannerPrivate {
        static QRegularExpression scanningRegex;
};

auto ClangLogScannerPrivate::scanningRegex = QRegularExpression("(.+):(\\d+):(\\d+): (.+?): (.+)");

ClangLogScanner::ClangLogScanner(QObject* parent) :
    LogScanner{parent} {
}

QList<BuildJob::BuildIssue> ClangLogScanner::scanLine(QString issues) {
    auto match = ClangLogScannerPrivate::scanningRegex.match(issues.trimmed());
    if (!match.hasMatch()) return {};

    QString file = match.captured(1);
    QString line = match.captured(2);
    QString col = match.captured(3);
    QString type = match.captured(4);
    QString message = match.captured(5);

    BuildJob::BuildIssue issue;
    if (type == "warning") {
        issue.issueType = BuildJob::BuildIssue::Warning;
    } else if (type == "error") {
        issue.issueType = BuildJob::BuildIssue::Error;
    } else {
        issue.issueType = BuildJob::BuildIssue::Informational;
    }
    issue.message = message;
    issue.file = file;
    issue.line = line.toInt();
    issue.col = col.toInt();

    return {issue};
}
