#ifndef CLANGLOGSCANNER_H
#define CLANGLOGSCANNER_H

#include <project/logscanner.h>

class ClangLogScanner : public LogScanner {
        Q_OBJECT
    public:
        explicit ClangLogScanner(QObject* parent = nullptr);

    signals:

        // LogScanner interface
    public:
        QList<BuildJob::BuildIssue> scanLine(QString issues);
};

#endif // CLANGLOGSCANNER_H
