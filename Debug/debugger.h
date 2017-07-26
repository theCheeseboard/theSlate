#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QObject>

class Debugger : public QObject
{
    Q_OBJECT
public:
    explicit Debugger(QObject *parent = nullptr);

signals:

public slots:
    virtual void startDebugging() = 0;
};

#endif // DEBUGGER_H
