#include "logscannermanager.h"

#include "logscanner.h"

struct LogScannerManagerPrivate {
        QList<LogScanner*> scanners;
};

LogScannerManager::LogScannerManager(QObject* parent) :
    QObject{parent} {
    d = new LogScannerManagerPrivate();
}

LogScannerManager::~LogScannerManager() {
    delete d;
}

void LogScannerManager::registerLogScanner(LogScanner* scanner) {
    d->scanners.append(scanner);
}

QList<BuildJob::BuildIssue> LogScannerManager::scanLine(QString line) {
    QList<BuildJob::BuildIssue> issues;
    for (auto* scanner : d->scanners) {
        issues.append(scanner->scanLine(line));
    }
    return issues;
}
