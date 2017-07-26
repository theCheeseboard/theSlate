#ifndef NODEJSDEBUGGER_H
#define NODEJSDEBUGGER_H

#include "debugger.h"
#include <QTcpSocket>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>

class NodeJsDebugger : public Debugger
{
    Q_OBJECT
public:
    explicit NodeJsDebugger(int port, QObject *parent = nullptr);

signals:

public slots:
    void startDebugging();

private slots:
    void dataAvailable();
    void sendCommand(QString command, QJsonObject params = QJsonObject());

private:
    QTcpSocket* socket;
    int retries = 0;
    int port;

    int currentId = 1;
};

#endif // NODEJSDEBUGGER_H
