#include "languageserverprocess.h"
#include "languageserverexception.h"
#include "tlogger.h"
#include <QCoroProcess>
#include <QCoroSignal>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMultiMap>

struct LanguageServerProcessPrivate {
        QString languageServerType;
        quint64 nextId = 0;

        qint64 bytesRemain = 0;
        QByteArray currentReadBuffer;

        QMultiMap<QUrl, LanguageServerProcess::Diagnostic> diagnostics;

        QJsonObject serverCapabilities;
        static QJsonObject clientCapabilities;
};

QJsonObject LanguageServerProcessPrivate::clientCapabilities = {
    {"workspace",    QJsonObject({{"workspaceFolders", true}})                                     },
    {"textDocument", QJsonObject({{"synchronization", QJsonObject({{"dynamicRegistration", false},
                                                          {"willSave", false},
                                                          {"willSaveWaitUntil", false},
                                                          {"didSave", false}})},
                         {"publishDiagnostics", QJsonObject({{"relatedInformation", false},
                                                    {"versionSupport", false},
                                                    {"dataSupport", false}})}})}
};

LanguageServerProcess::LanguageServerProcess(QString languageServerType, QObject* parent) :
    QProcess(parent) {
    d = new LanguageServerProcessPrivate();
    d->languageServerType = languageServerType;

    connect(this, &QProcess::readyRead, this, [this] {
        Q_FOREVER {
            if (d->bytesRemain == 0) {
                // Reading headers
                char c;
                bool ok = this->getChar(&c);
                if (!ok) return;

                d->currentReadBuffer.append(c);

                if (d->currentReadBuffer.endsWith("\r\n\r\n")) {
                    // Parse the headers
                    auto headers = QString(d->currentReadBuffer).split("\r\n", Qt::SkipEmptyParts);
                    for (auto header : headers) {
                        if (header.startsWith("Content-Length:", Qt::CaseInsensitive)) {
                            d->bytesRemain = header.mid(15).trimmed().toLongLong();
                        }
                    }

                    d->currentReadBuffer.clear();
                }
            } else {
                auto buf = this->read(d->bytesRemain);
                d->bytesRemain -= buf.length();
                d->currentReadBuffer.append(buf);

                if (d->bytesRemain != 0) return;

                QJsonObject rootObject = QJsonDocument::fromJson(d->currentReadBuffer).object();
                tDebug("LanguageServerProcess") << "<-- " << QString(QJsonDocument(rootObject).toJson());

                d->currentReadBuffer.clear();

                if (rootObject.contains("method")) {
                    emit jsonRpcNotification(rootObject);
                    handleJsonRpcNotification(rootObject);
                } else {
                    emit jsonRpcResponse(rootObject);
                }
            }
        }
    });
}

LanguageServerProcess::~LanguageServerProcess() {
    delete d;
}

QList<QChar> LanguageServerProcess::completionTriggerCharacters() {
    QList<QChar> triggerCharacters;
    auto triggers = d->serverCapabilities.value("completionProvider").toObject().value("triggerCharacters").toArray();
    for (auto trigger : triggers) {
        auto triggerStr = trigger.toString();
        if (triggerStr.length() >= 1) triggerCharacters.append(triggerStr.at(0));
    }
    return triggerCharacters;
}

QList<QChar> LanguageServerProcess::completionCommitCharacters() {
    QList<QChar> commitCharacters;
    auto commitCharactersArray = d->serverCapabilities.value("completionProvider").toObject().value("allCommitCharacters").toArray();
    for (auto commitCharacter : commitCharactersArray) {
        auto commitCharacterStr = commitCharacter.toString();
        if (commitCharacterStr.length() >= 1) commitCharacters.append(commitCharacterStr.at(0));
    }
    return commitCharacters;
}

bool LanguageServerProcess::prefersIncrementalSync() {
    return d->serverCapabilities.value("textDocumentSync").toObject().value("change").toInt() == 2;
}

QCoro::Task<> LanguageServerProcess::startLanguageServer(QJsonObject extraInitialisationOptions) {
    // TODO: Make LSPs configurable
    if (this->state() != QProcess::NotRunning) {
        this->kill();

        co_await qCoro(this).waitForFinished();

        // Clear the state
        auto newD = new LanguageServerProcessPrivate();
        newD->languageServerType = d->languageServerType;
        delete d;
        d = newD;
    }

    if (d->languageServerType != "clangd") {
        throw LanguageServerException(-32098, "Unknown Language Server", QJsonValue());
    }

    this->start("clangd");

    try {
        if (!co_await qCoro(this).waitForStarted()) {
            throw LanguageServerException(-32097, "Incompatible Language Server", QJsonValue());
        }

        auto initOptions = QJsonObject({
            {"processId",        QCoreApplication::applicationPid()                                           },
            {"clientInfo",       QJsonObject({{"name", QCoreApplication::applicationName()},
                               {"version", QCoreApplication::applicationVersion()}})},
            {"locale",           QLocale().name()                                                             },
            {"capabilities",     d->clientCapabilities                                                        },
            {"workspaceFolders", QJsonValue()                                                                 }
        });

        for (auto optionKey : extraInitialisationOptions.keys()) {
            initOptions.insert(optionKey, extraInitialisationOptions.value(optionKey));
        }

        auto initializeResult = co_await this->call("initialize", initOptions);

        d->serverCapabilities = initializeResult.toObject().value("capabilities").toObject();

        this->notify("initialized", QJsonObject());
    } catch (LanguageServerException& ex) {
        this->kill();
        throw LanguageServerException(-32097, "Incompatible Language Server", QJsonValue());
    }
}

QString LanguageServerProcess::serverTypeForFileName(QString fileName) {
    // TODO: Refactor into configuration file
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix() == "cpp" || fileInfo.suffix() == "hpp" || fileInfo.suffix() == "c" || fileInfo.suffix() == "h" || fileInfo.suffix() == "m" || fileInfo.suffix() == "mm" || fileInfo.suffix() == "cc") {
        return "clangd";
    }
    return "";
}

QCoro::Task<QJsonValue> LanguageServerProcess::call(QString method, QJsonValue params) {
    if (this->state() != QProcess::Running) {
        throw LanguageServerException(-32099, "Language Server not running", QJsonValue());
    }

    auto requestId = QString::number(++d->nextId);

    QJsonObject rootObject;
    rootObject.insert("jsonrpc", "2.0");
    rootObject.insert("id", requestId);
    rootObject.insert("method", method);
    rootObject.insert("params", params);

    writeJsonRpc(rootObject);

    Q_FOREVER {
        auto response = co_await qCoro(this, &LanguageServerProcess::jsonRpcResponse);
        if (response.value("id").toString() != requestId) continue;

        if (response.contains("result")) {
            co_return response.value("result");
        } else {
            auto error = response.value("error").toObject();
            throw LanguageServerException(error.value("code").toInt(), error.value("message").toString(), error.value("data"));
        }
    }
}

void LanguageServerProcess::notify(QString method, QJsonValue params) {
    if (this->state() != QProcess::Running) {
        throw LanguageServerException(-32099, "Language Server not running", QJsonValue());
    }

    QJsonObject rootObject;
    rootObject.insert("jsonrpc", "2.0");
    rootObject.insert("method", method);
    rootObject.insert("params", params);

    writeJsonRpc(rootObject);
}

void LanguageServerProcess::addWorkspaceFolder(QUrl documentUri, QString name) {
    this->notify("workspace/didChangeWorkspaceFolders", QJsonObject({
                                                            {"event", QJsonObject({{"added", QJsonArray({QJsonObject({{"uri", documentUri.toString()},
                                                                                                 {"name", name}})})},
                                                                          {"removed", QJsonArray()}})}
    }));
}

void LanguageServerProcess::didOpen(QUrl documentUri, QString languageId, int version, QString text) {
    this->notify("textDocument/didOpen", QJsonObject({
                                             {"textDocument", QJsonObject({{"uri", documentUri.toString()},
                                                                  {"languageId", languageId},
                                                                  {"version", version},
                                                                  {"text", text}})}
    }));
}

void LanguageServerProcess::didChange(QUrl documentUri, int version, QString text) {
    this->notify("textDocument/didChange", QJsonObject({
                                               {"textDocument",   QJsonObject({{"uri", documentUri.toString()},
                                                                    {"version", version}})},
                                               {"contentChanges", QJsonArray({QJsonObject({{"text", text}})})                                            }
    }));
}

void LanguageServerProcess::didChange(QUrl documentUri, int version, QList<TextDocumentContentChangeEvent> events) {
    QJsonArray changes;

    for (auto event : events) {
        QJsonObject change;
        change.insert("text", event.text);

        QJsonObject range;
        range.insert("start", QJsonObject({
                                  {"line",      event.start.y()},
                                  {"character", event.start.x()}
        }));
        range.insert("end", QJsonObject({
                                {"line",      event.end.y()},
                                {"character", event.end.x()}
        }));
        change.insert("range", range);

        changes.append(change);
    }

    this->notify("textDocument/didChange", QJsonObject({
                                               {"textDocument",   QJsonObject({{"uri", documentUri.toString()},
                                                                    {"version", version}})},
                                               {"contentChanges", changes                                                                                }
    }));
}

void LanguageServerProcess::didClose(QUrl documentUri) {
    this->notify("textDocument/didClose", QJsonObject({
                                              {"textDocument", QJsonObject({{"uri", documentUri.toString()}})}
    }));
}

QList<LanguageServerProcess::Diagnostic> LanguageServerProcess::diagnostics(QUrl url) {
    return d->diagnostics.values(url);
}

void LanguageServerProcess::writeJsonRpc(QJsonObject object) {
    auto data = QJsonDocument(object).toJson();
    tDebug("LanguageServerProcess") << "--> " << QString(data);

    QStringList headers = {
        QStringLiteral("Content-Length: %1").arg(data.size()),
        QStringLiteral("application/vscode-jsonrpc; charset=utf-8"),
        "", "" // Insert two empty "headers" for the two new lines
    };

    QByteArray payload = headers.join("\r\n").toUtf8() + data;

    this->write(payload);
}

void LanguageServerProcess::handleJsonRpcNotification(QJsonObject notification) {
    auto method = notification.value("method");
    auto params = notification.value("params").toObject();
    if (method == "textDocument/publishDiagnostics") {
        // TODO: Interpret diagnostics
        auto url = QUrl(params.value("uri").toString());
        d->diagnostics.remove(url);

        auto diagnosticsArray = params.value("diagnostics").toArray();
        for (auto diagnosticValue : diagnosticsArray) {
            auto diagnostic = diagnosticValue.toObject();
            auto dg = Diagnostic();
            dg.code = diagnostic.value("code").toString();
            dg.message = diagnostic.value("message").toString().split("\n").first().trimmed();
            dg.severity = static_cast<Diagnostic::Severity>(diagnostic.value("severity").toInt());

            auto range = diagnostic.value("range").toObject();
            dg.start = QPoint(range.value("start").toObject().value("character").toInt(), range.value("start").toObject().value("line").toInt());
            dg.end = QPoint(range.value("end").toObject().value("character").toInt(), range.value("end").toObject().value("line").toInt());
            d->diagnostics.insert(url, dg);
        }

        emit publishDiagnostics(url);
    }
}
