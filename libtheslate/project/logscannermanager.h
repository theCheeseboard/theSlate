#ifndef LOGSCANNERMANAGER_H
#define LOGSCANNERMANAGER_H

#include "buildjob.h"
#include <QObject>

class LogScanner;
struct LogScannerManagerPrivate;
class LogScannerManager : public QObject {
        Q_OBJECT
    public:
        explicit LogScannerManager(QObject* parent = nullptr);
        ~LogScannerManager();

        void registerLogScanner(LogScanner* scanner);

        QList<BuildJob::BuildIssue> scanLine(QString line);

    signals:

    private:
        LogScannerManagerPrivate* d;
};

#endif // LOGSCANNERMANAGER_H
