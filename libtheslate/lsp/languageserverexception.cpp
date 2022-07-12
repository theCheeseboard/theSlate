#include "languageserverexception.h"
#include <tlogger.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct LanguageServerExceptionPrivate {
	int code;
	QString message;
	QJsonValue data;
};

LanguageServerException::LanguageServerException(int code, QString message, QJsonValue data)
{
	d = new LanguageServerExceptionPrivate();
	d->code = code;
	d->message = message;
	d->data = data;

	QString dataStr;
	if (data.isObject()) {
		dataStr = QJsonDocument(data.toObject()).toJson();
	} else if (data.isArray()) {
		dataStr = QJsonDocument(data.toArray()).toJson();
	} else if (data.isString()) {
		dataStr = data.toString();
	} else if (data.isDouble()) {
		dataStr = QString::number(data.toDouble());
	} else if (data.isNull()) {
		dataStr = "null";
	}
	tWarn("LanguageServerException") << "code: " << code << "\nmessage: " << message << "\ndata: " << dataStr;
}

LanguageServerException::LanguageServerException(const LanguageServerException& other)
{
	d = new LanguageServerExceptionPrivate();
	d->code = other.d->code;
	d->message = other.d->message;
	d->data = other.d->data;
}

LanguageServerException::~LanguageServerException()
{
	delete d;
}

void LanguageServerException::raise() const {
	throw* this;
}

inline LanguageServerException* LanguageServerException::clone() const {
	return new LanguageServerException(*this);
}

int LanguageServerException::code()
{
	return d->code;
}

QString LanguageServerException::message()
{
	return d->message;
}

QJsonValue LanguageServerException::data()
{
	return d->data;
}
