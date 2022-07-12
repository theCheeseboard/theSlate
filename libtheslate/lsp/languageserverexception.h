#ifndef LANGUAGESERVEREXCEPTION_H
#define LANGUAGESERVEREXCEPTION_H

#include <QException>
#include <QJsonValue>
#include "libtheslate_global.h"

struct LanguageServerExceptionPrivate;
class LIBTHESLATE_EXPORT LanguageServerException : public QException {
public:
	LanguageServerException(int code, QString message, QJsonValue data);
	LanguageServerException(const LanguageServerException& other);
	~LanguageServerException();

	void raise() const override;
	LanguageServerException* clone() const override;

	int code();
	QString message();
	QJsonValue data();

private:
	LanguageServerExceptionPrivate* d;
};

#endif // LANGUAGESERVEREXCEPTION_H