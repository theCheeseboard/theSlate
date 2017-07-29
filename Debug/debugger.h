#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QObject>
#include "exception.h"

class Debugger : public QObject
{
    Q_OBJECT
public:
    explicit Debugger(QObject *parent = nullptr);

signals:
    void lineHit(QString file, int lineNumber);
    void paused();
    void unpaused();
    void exceptionEncountered(Exception exception, QString file);
    void loadFile(QString file);
    void loadFakeFile(QString filename, QString contents);

public slots:
    virtual void startDebugging() = 0;
    virtual void cont() = 0;
    virtual void pause() = 0;
    virtual void kill() = 0;
    virtual void stepOver() = 0;
    virtual void stepIn() = 0;
    virtual void stepOut() = 0;

    virtual void setBreakpoint(QString file, int line) = 0;
    virtual void clearBreakpoint(QString file, int line) = 0;
};

#endif // DEBUGGER_H
