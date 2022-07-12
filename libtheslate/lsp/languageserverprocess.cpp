#include "languageserverprocess.h"
#include "tlogger.h"
#include "languageserverexception.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoroSignal>
#include <QCoroProcess>

struct LanguageServerProcessPrivate {
	quint64 nextId = 0;

	qint64 bytesRemain = 0;
	QByteArray currentReadBuffer;

	QJsonObject serverCapabilities;
	static QJsonObject clientCapabilities;
};

QJsonObject LanguageServerProcessPrivate::clientCapabilities = {
	{"workspace", QJsonObject({
		{"workspaceFolders", true}
	})}
};

LanguageServerProcess::LanguageServerProcess(QObject* parent) : QProcess(parent) {
	d = new LanguageServerProcessPrivate();

	connect(this, &QProcess::readyRead, this, [this] {
		Q_FOREVER {
			if (d->bytesRemain == 0) {
				//Reading headers
				char c;
				bool ok = this->getChar(&c);
				if (!ok) return;

				d->currentReadBuffer.append(c);

				if (d->currentReadBuffer.endsWith("\r\n\r\n")) {
					//Parse the headers
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

				emit jsonRpcResponse(rootObject);
			}
		}
	});
}

LanguageServerProcess::~LanguageServerProcess() {
	delete d;
}

QCoro::Task<> LanguageServerProcess::startLanguageServer(QString serverType)
{
	//TODO: Make LSPs configurable

	if (serverType != "clangd") {
		throw LanguageServerException(-32098, "Unknown Language Server", QJsonValue());
	}

	this->start("clangd");

	try {
		if (!co_await qCoro(this).waitForStarted()) {
			throw LanguageServerException(-32097, "Incompatible Language Server", QJsonValue());
		}

		auto initializeResult = co_await this->call("initialize", QJsonObject({
			{"processId", QCoreApplication::applicationPid()},
			{"clientInfo", QJsonObject({
				{"name", QCoreApplication::applicationName()},
				{"version", QCoreApplication::applicationVersion()}
			})},
			{"locale", QLocale().name()},
			{"capabilities", d->clientCapabilities},
			{"workspaceFolders", QJsonValue()}
		}));

		d->serverCapabilities = initializeResult.toObject().value("capabilities").toObject();

		this->notify("initialized", QJsonObject());
	} catch (LanguageServerException& ex) {
		this->kill();
		throw LanguageServerException(-32097, "Incompatible Language Server", QJsonValue());
	}
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

void LanguageServerProcess::notify(QString method, QJsonValue params)
{
	if (this->state() != QProcess::Running) {
		throw LanguageServerException(-32099, "Language Server not running", QJsonValue());
	}

	QJsonObject rootObject;
	rootObject.insert("jsonrpc", "2.0");
	rootObject.insert("method", method);
	rootObject.insert("params", params);

	writeJsonRpc(rootObject);
}

void LanguageServerProcess::addWorkspaceFolder(QUrl documentUri, QString name)
{
	this->notify("workspace/didChangeWorkspaceFolders", QJsonObject({
		{"event", QJsonObject({
			{"added", QJsonArray({
				QJsonObject({
					{"uri", documentUri.toString()},
					{"name", name}
				})
			})},
			{"removed", QJsonArray()}
		})}
	}));
}

void LanguageServerProcess::writeJsonRpc(QJsonObject object) {
	auto data = QJsonDocument(object).toJson();
	tDebug("LanguageServerProcess") << "--> " << QString(data);

	QStringList headers = {
		QStringLiteral("Content-Length: %1").arg(data.size()),
		QStringLiteral("application/vscode-jsonrpc; charset=utf-8"),
		"", "" //Insert two empty "headers" for the two new lines
	};

	QByteArray payload = headers.join("\r\n").toUtf8() + data;

	this->write(payload);
}
