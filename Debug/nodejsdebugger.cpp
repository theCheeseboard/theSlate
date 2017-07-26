#include "nodejsdebugger.h"

NodeJsDebugger::NodeJsDebugger(int port, QObject *parent) : Debugger(parent)
{
    socket = new QTcpSocket();
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    this->port = port;
}

void NodeJsDebugger::startDebugging() {
    QEventLoop loop;

    QNetworkAccessManager mgr;
    mgr.setNetworkAccessible(QNetworkAccessManager::Accessible);

    QNetworkRequest infoRequest(QUrl("http://localhost:" + QString::number(port) + "/json"));
    QNetworkReply* infoReply = mgr.get(infoRequest);
    connect(infoReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200) {
        QByteArray request = infoReply->readAll();

        QJsonDocument doc = QJsonDocument::fromJson(request);
        QString id = doc.array().first().toObject().value("id").toString();

        /*
        //Create WebSockets request to ID
        QNetworkRequest wsRequest(QUrl("http://localhost:" + QString::number(port) + "/" + id));
        wsRequest.setRawHeader("Upgrade", "websocket");
        wsRequest.setRawHeader("Connection", "Upgrade");
        wsRequest.setRawHeader("Sec-Websocket-Key", QByteArray("akfdgakjhgawkjhfegkajwhef").toBase64());
        wsRequest.setRawHeader("Sec-WebSocket-Version", "13");
        wsRequest.setRawHeader("Sec-WebSocket-Protocol", "nodejs");
        wsRequest.setRawHeader("Origin", "127.0.0.1");
        QNetworkReply* wsReply = mgr.get(wsRequest);
        connect(wsReply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        qDebug() << infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug() << infoReply->readAll();

        if (infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200) {
            qDebug() << "HTTP OK!";
        } else if (infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 101) {
            qDebug() << "Switching protocols!";
        }

        //socket->connectToHost("localhost", port);*/

        //Create socket
        socket->connectToHost("localhost", port);
        socket->write(QString("GET /" + id + " HTTP/1.1\r\n").toUtf8());
        socket->write("Upgrade: websocket\r\n");
        socket->write("Connection: Upgrade\r\n");
        socket->write("Sec-WebSocket-Key: " + QByteArray("akfdgakjhgawkjhfegkajwhef").toBase64() + "\r\n");
        socket->write("Sec-WebSocket-Version: 13\r\n");
        socket->write("Sec-WebSocket-Protocol: nodejs\r\n");
        socket->write("Origin: 127.0.0.1\r\n");
        socket->write("\r\n");
    } else {
        qDebug() << "Server returned " + infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        this->deleteLater();
    }
}

void NodeJsDebugger::dataAvailable() {
    QString response = socket->readAll();
    qDebug() << response;
    if (response.startsWith("HTTP/1.1 101 Switching Protocols")) { //Handshake success!
        //socket->write("{\"command\":\"evaluate\",\"arguments\":{\"expression\":\"process.pid\",\"global\":true},\"type\":\"request\",\"seq\":1}");
        sendCommand("Runtime.enable");
        sendCommand("Profiler.enable");
        {
            QJsonObject obj;
            obj.insert("interval", 100);
            sendCommand("Runtime.setSamplingInterval", obj);
        }

        sendCommand("Debugger.enable");
        {
            QJsonObject obj;
            obj.insert("state", "none");
            sendCommand("Debugger.setPauseOnExceptions", obj);
        }
        {
            QJsonObject obj;
            obj.insert("maxDepth", 0);
            sendCommand("Debugger.setAsyncCallStackDepth", obj);
        }
        {
            QJsonObject obj;
            obj.insert("patterns", QJsonArray());
            sendCommand("Debugger.setBlackboxPatterns", obj);
        }
        {
            QJsonObject obj;
            obj.insert("state", "none");
            sendCommand("Debugger.setPauseOnExceptions", obj);
        }
        sendCommand("Runtime.runIfWaitingForDebugger");
    }
}

void NodeJsDebugger::sendCommand(QString command, QJsonObject params) {
    QJsonObject rootJsonObject;
    rootJsonObject.insert("id", currentId);
    rootJsonObject.insert("method", command);

    if (!params.isEmpty()) {
        rootJsonObject.insert("params", params);
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootJsonObject);
    QByteArray data = jsonDoc.toJson();
    int dataLength = data.length();

    int singleByteLength;
    QByteArray additionalLength;
    if (dataLength > 0xFFFF) {
        singleByteLength = 127;
        int remaining = dataLength;
        additionalLength.fill(0x0, 8);
        for (int i = 0; i < 8; i++) {
            additionalLength.replace(7 - i, remaining & 0xFF);
            remaining >>= 8;
        }
    } else if (dataLength > 125) {
        singleByteLength = 126;
        additionalLength.append((dataLength & 0xFF00) >> 8);
        additionalLength.append(dataLength & 0xFF);
    } else {
        singleByteLength = dataLength;
    }

    QByteArray header;
    header.append(0x80 | 0x1 | 0x80 | singleByteLength);

    QByteArray mask;
    mask.fill(0x0, 4);

    QByteArray masked;
    masked.fill(0x0, dataLength);

    for (int i = 0; i < dataLength; i++) {
        masked.replace(i, data.at(i) ^ mask.at(i % 4));
    }

    currentId++;

    QByteArray payload = header.append(additionalLength).append(mask).append(masked);

    socket->write(data);
}
