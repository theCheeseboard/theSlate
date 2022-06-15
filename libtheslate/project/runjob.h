#ifndef RUNJOB_H
#define RUNJOB_H

#include <QObject>

class RunJob : public QObject {
        Q_OBJECT
    public:
        explicit RunJob(QObject* parent = nullptr);

        virtual void start() = 0;

    signals:
};

typedef QSharedPointer<RunJob> RunJobPtr;

#endif // RUNJOB_H
