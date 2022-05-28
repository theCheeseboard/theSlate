#ifndef LOGSCANNER_H
#define LOGSCANNER_H

#include "buildjob.h"
#include <QObject>

class LogScanner : public QObject {
        Q_OBJECT
    public:
        explicit LogScanner(QObject* parent = nullptr);

        virtual QList<BuildJob::BuildIssue> scanLine(QString issues) = 0;

    signals:
};

#endif // LOGSCANNER_H
