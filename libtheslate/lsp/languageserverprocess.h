#ifndef LANGUAGESERVERPROCESS_H
#define LANGUAGESERVERPROCESS_H

#include <QProcess>
#include <QCoroTask>
#include <QJsonObject>
#include "libtheslate_global.h"

struct LanguageServerProcessPrivate;
class LIBTHESLATE_EXPORT LanguageServerProcess : public QProcess {
	Q_OBJECT

public:
	explicit LanguageServerProcess(QObject* parent = nullptr);
	~LanguageServerProcess();

	QCoro::Task<> startLanguageServer(QString serverType);

	QCoro::Task<QJsonValue> call(QString method, QJsonValue params);
	void notify(QString method, QJsonValue params);

	void addWorkspaceFolder(QUrl documentUri, QString name);

signals:
	void jsonRpcResponse(QJsonObject response);

private:
	LanguageServerProcessPrivate* d;

	void writeJsonRpc(QJsonObject object);
};

#endif // LANGUAGESERVERPROCESS_H