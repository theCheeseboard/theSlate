#include "nodejsdebugger.h"

NodeJsDebugger::NodeJsDebugger(int port, QObject *parent) : Debugger(parent)
{
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

        //Create WebSockets request to ID
        QNetworkRequest wsRequest(QUrl("ws://127.0.0.1:" + QString::number(port) + "/" + id));
        wsRequest.setRawHeader("Sec-WebSocket-Protocol", "nodejs");

        ws = new QWebSocket("127.0.0.1", QWebSocketProtocol::Version13);
        connect(ws, SIGNAL(textMessageReceived(QString)), this, SLOT(dataAvailable(QString)));
        connect(ws, &QWebSocket::connected, [=] {
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
                obj.insert("state", "uncaught");
                sendCommand("Debugger.setPauseOnExceptions", obj);
            }
            sendCommand("Runtime.runIfWaitingForDebugger");
        });
        connect(ws, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            [=](QAbstractSocket::SocketError error){
            qDebug() << error;
        });
        connect(ws, &QWebSocket::stateChanged, [=](QAbstractSocket::SocketState state) {
            qDebug() << state;
        });
        ws->open(wsRequest);
    } else {
        qDebug() << "Server returned " + infoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        this->deleteLater();
    }
}

void NodeJsDebugger::dataAvailable(QString frame) {
    qDebug() << frame;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(frame.toUtf8());
    QJsonObject rootObj = jsonDoc.object();

    if (rootObj.contains("id")) {
        QString id = QString::number(rootObj.value("id").toInt());
        QJsonObject result = rootObj.value("result").toObject();

        if (breakpoints.keys().contains(id)) {
            Breakpoint bp = breakpoints.take(id);
            bp.id = result.value("breakpointId").toString();
            breakpoints.insert(result.value("breakpointId").toString(), bp);
        } else {
            for (QString key : loadedScripts.keys()) {
                DebuggerScript script = loadedScripts.value(key);
                if (script.pendingContents == id.toInt()) {
                    script.contents = result.value("scriptSource").toString();
                    script.pendingContents = -1;
                    loadedScripts.insert(key, script);
                }
            }
        }
    } else if (rootObj.contains("method")) {
        QString method = rootObj.value("method").toString();
        if (method == "Debugger.paused") { //Paused Execution!
            QJsonObject params = rootObj.value("params").toObject();
            emit paused();

            if (params.value("reason") == "exception") {
                //Determine location
                QJsonObject callFrame = params.value("callFrames").toArray().first().toObject();
                QJsonObject location = callFrame.value("location").toObject();
                QString scriptId = location.value("scriptId").toString();
                int lineNumber = location.value("lineNumber").toInt() + 1;

                DebuggerScript script = loadedScripts.value(scriptId);

                QJsonObject data = params.value("data").toObject();

                Exception exception;
                exception.errorType = data.value("className").toString();
                exception.description = data.value("description").toString();
                exception.line = lineNumber;

                if (!script.url.startsWith("/")) {
                    emit loadFakeFile(script.url, script.contents);
                }
                emit exceptionEncountered(exception, script.url);
                emit lineHit(script.url, lineNumber);

            } else {
                //Determine location
                QJsonObject callFrame = params.value("callFrames").toArray().first().toObject();
                QJsonObject location = callFrame.value("location").toObject();
                QString scriptId = location.value("scriptId").toString();
                int lineNumber = location.value("lineNumber").toInt() + 1;

                DebuggerScript script = loadedScripts.value(scriptId);
                if (script.url.startsWith("/")) {
                    emit lineHit(script.url, lineNumber);
                } else {
                    emit loadFakeFile(script.url, script.contents);
                    emit lineHit(script.url, lineNumber);
                }
            }
        } else if (method == "Debugger.resumed") {
            emit unpaused();
        } else if (method == "Debugger.scriptParsed") {
            QJsonObject params = rootObj.value("params").toObject();

            DebuggerScript script;
            script.id = params.value("scriptId").toString();
            script.url = params.value("url").toString();

            QJsonObject replyParams;
            replyParams.insert("scriptId", script.id);
            sendCommand("Debugger.getScriptSource", replyParams);

            script.pendingContents = currentId;

            loadedScripts.insert(params.value("scriptId").toString(), script);
        } else if (method == "Runtime.executionContextDestroyed") {
            QJsonObject params = rootObj.value("params").toObject();
            if (params.value("executionContextId").toInt() == 1) {
                QTimer::singleShot(1000, [=] {
                    ws->close();
                    this->deleteLater();
                });
            }
        }
    }
}

void NodeJsDebugger::sendCommand(QString command, QJsonObject params) {
    currentId++;
    QJsonObject rootJsonObject;
    rootJsonObject.insert("id", currentId);
    rootJsonObject.insert("method", command);

    if (!params.isEmpty()) {
        rootJsonObject.insert("params", params);
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootJsonObject);
    QByteArray data = jsonDoc.toJson();

    ws->sendTextMessage(data);
}

void NodeJsDebugger::cont() {
    sendCommand("Debugger.resume");
}

void NodeJsDebugger::stepIn() {
    sendCommand("Debugger.stepInto");
}

void NodeJsDebugger::stepOut() {
    sendCommand("Debugger.stepOut");
}

void NodeJsDebugger::stepOver() {
    sendCommand("Debugger.stepOver");
}

void NodeJsDebugger::setBreakpoint(QString file, int line) {
    QJsonObject params;
    QJsonObject location;

    QString scriptId;

    for (DebuggerScript script : loadedScripts.values()) {
        if (script.url == file) {
            scriptId = script.id;
        }
    }

    location.insert("scriptId", scriptId);
    location.insert("lineNumber", line - 1);
    params.insert("location", location);

    sendCommand("Debugger.setBreakpoint", params);

    Breakpoint bp;
    bp.file = file;
    bp.line = line;
    breakpoints.insert(QString::number(currentId), bp);
}

void NodeJsDebugger::clearBreakpoint(QString file, int line) {
    QJsonObject params;

    for (Breakpoint bp : breakpoints.values()) {
        if (bp.file == file && bp.line == line) {
            params.insert("breakpointId", bp.id);
            breakpoints.remove(bp.id);
        }
    }

    sendCommand("Debugger.removeBreakpoint", params);
}

void NodeJsDebugger::pause() {
    sendCommand("Debugger.pause");
}

void NodeJsDebugger::kill() {

}
