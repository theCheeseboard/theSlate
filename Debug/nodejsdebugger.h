#ifndef NODEJSDEBUGGER_H
#define NODEJSDEBUGGER_H

#include "debugger.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QWebSocket>
#include <QTimer>
#include "exception.h"

class NodeJsDebugger : public Debugger
{
    Q_OBJECT

    struct DebuggerScript {
        QString id;
        QString url;
        QString contents;
        int pendingContents = -1;
    };

    struct Breakpoint {
        QString id;
        QString file;
        int line;
    };

public:
    explicit NodeJsDebugger(int port, QObject *parent = nullptr);

signals:

public slots:
    void startDebugging();
    void cont();
    void pause();
    void kill();
    void stepIn();
    void stepOver();
    void stepOut();

    void setBreakpoint(QString file, int line);
    void clearBreakpoint(QString file, int line);

private slots:
    void dataAvailable(QString frame);
    void sendCommand(QString command, QJsonObject params = QJsonObject());

private:
    QWebSocket* ws;
    int retries = 0;
    int port;

    int currentId = 0;
    QMap<QString, DebuggerScript> loadedScripts;
    QMap<QString, Breakpoint> breakpoints;
};

#endif // NODEJSDEBUGGER_H
