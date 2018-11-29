#include "httpbackend.h"
#include <QStorageInfo>
#include <QNetworkReply>
#include <QAuthenticator>
#include "httpbasicauthdialog.h"

HttpBackend::HttpBackend(QUrl url, QObject *parent) : FileBackend(parent)
{
    this->fileUrl = url;
}

tPromise<void>* HttpBackend::save(QByteArray fileContents) {
    return new tPromise<void>([=](QString &error) {
        //For now don't allow saving
        QNetworkAccessManager mgr;
        //mgr.put()
        error = "Can't save HTTP file";
    });
}

tPromise<QByteArray>* HttpBackend::load() {
    if (currentAuthRealm != "") {
        QEventLoop* loop = new QEventLoop();
        HttpBasicAuthDialog* authDialog = new HttpBasicAuthDialog();
        //openDialog->setWindowFlag(Qt::Sheet);
        authDialog->setText(tr("Log in to %1").arg(currentAuthHost) + "\n" + tr("Server Realm: %1").arg(currentAuthRealm));
        authDialog->setWindowModality(Qt::WindowModal);

        connect(authDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        authDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (authDialog->result() == QDialog::Accepted) {
            currentAuthUsername = authDialog->username();
            currentAuthPassword = authDialog->password();
        }
        authDialog->deleteLater();
    }

    currentAuthRealm = "";
    return new tPromise<QByteArray>([=](QString& error) {
        QNetworkAccessManager mgr;

        QNetworkRequest req(fileUrl);
        req.setHeader(QNetworkRequest::UserAgentHeader, "theSlate/1.0");
        if (redirect) req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

        QNetworkReply* reply;
        reply = mgr.get(req);

        QList<QSslError>* sslErrors = new QList<QSslError>();

        QEventLoop* loop = new QEventLoop();
        connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);
        connect(&mgr, &QNetworkAccessManager::authenticationRequired, [=](QNetworkReply* reply, QAuthenticator* authenticator) {
            if (currentAuthUsername != "" && currentAuthPassword != "") {
                authenticator->setUser(currentAuthUsername);
                authenticator->setPassword(currentAuthPassword);
            } else {
                currentAuthRealm = authenticator->realm();
                currentAuthHost = reply->url().host();
            }
        });
        connect(reply, &QNetworkReply::sslErrors, [=](QList<QSslError> errors) {
            sslErrors->append(errors);
        });
        loop->exec();

        loop->deleteLater();

        QTimer::singleShot(0, [=] {
            delete sslErrors;
        });

        switch (reply->error()) {
            case QNetworkReply::NoError:
            case QNetworkReply::ContentNotFoundError:
            case QNetworkReply::ContentGoneError:
            case QNetworkReply::InternalServerError:
            {
                int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (httpCode / 100 == 2) {
                    return reply->readAll();
                } else if (httpCode / 100 == 3) {
                    QString redirectExplanation;
                    QString httpCodeExplanation;

                    switch (httpCode) {
                        case 301:
                            httpCodeExplanation = tr("301: Moved Permanently");
                            break;
                        case 302:
                            httpCodeExplanation = tr("302: Found");
                            break;
                        case 303:
                            httpCodeExplanation = tr("303: See Other");
                            break;
                        case 307:
                            httpCodeExplanation = tr("307: Temporary Redirect");
                            break;
                        case 308:
                            httpCodeExplanation = tr("308: Permanent Redirect");
                            break;
                        default:
                            httpCodeExplanation = tr("HTTP status code %1").arg(QString::number(httpCode));
                    }

                    //redirectExplanation = tr("The server responded with %1").arg(httpCodeExplanation) + "\n\n";
                    //redirectExplanation = redirectExplanation.append(tr("The server wants to redirect you to %1")).arg(reply->header(QNetworkRequest::LocationHeader).toString());
                    redirectExplanation = httpCodeExplanation + "\n\n" + tr("The server wants to redirect you to the following URL:") + "\n" + reply->header(QNetworkRequest::LocationHeader).toString();
                    return redirectExplanation.toUtf8();
                } else {
                    QString explanation;

                    switch (httpCode) {
                        case 400:
                            explanation = tr("400: Bad Request");
                            break;
                        case 401:
                            explanation = tr("401: Unauthorized");
                            break;
                        case 402:
                            explanation = tr("402: Payment Required");
                            break;
                        case 403:
                            explanation = tr("403: Forbidden");
                            break;
                        case 404:
                            explanation = tr("404: Not Found");
                            break;
                        case 405:
                            explanation = tr("405: Method Not Allowed");
                            break;
                        case 410:
                            explanation = tr("410: Gone");
                            break;
                        case 418:
                            explanation = tr("418: I'm a teapot");
                            break;
                        case 500:
                            explanation = tr("500: Internal Server Error");
                            break;
                        case 501:
                            explanation = tr("501: Not Implemented");
                            break;
                        case 502:
                            explanation = tr("502: Bad Gateway");
                            break;
                        case 503:
                            explanation = tr("503: Service Unavailable");
                            break;
                        case 504:
                            explanation = tr("504: Gateway Timeout");
                            break;
                        default:
                            explanation = tr("HTTP status code %1").arg(QString::number(httpCode));
                    }

                    error = tr("Could not load resource because the server responded with %1").arg(explanation);

                    return QByteArray();
                }
            }
            case QNetworkReply::SslHandshakeFailedError: {
                if (sslErrors->count() == 0) {
                    error = tr("The SSL handshake failed and a secure connection to the server cannot be made.");
                    return QByteArray();
                }

                QSslError e = sslErrors->first();
                QStringList errorExplanation;

                errorExplanation.append(tr("An SSL error occurred trying to retrieve the resource."));
                errorExplanation.append("");
                errorExplanation.append(tr("SSL Error: ") + e.errorString());
                errorExplanation.append("");
                errorExplanation.append(tr("Certificate information is below:"));
                errorExplanation.append("");

                auto getSInfo = [](QStringList s) {
                    if (s.count() > 0) {
                        return s.first();
                    } else {
                        return tr("[Not part of certificate]");
                    }
                };

                QSslCertificate cert = e.certificate();
                errorExplanation.append(tr("ISSUED TO:"));
                errorExplanation.append(tr("Common Name (CN)") + "          " + getSInfo(cert.subjectInfo(QSslCertificate::CommonName)));
                errorExplanation.append(tr("Organization (O)") + "          " + getSInfo(cert.subjectInfo(QSslCertificate::Organization)));
                errorExplanation.append(tr("Organizational Unit (OU)") + "  " + getSInfo(cert.subjectInfo(QSslCertificate::Organization)));
                errorExplanation.append("");
                errorExplanation.append(tr("ISSUED BY:"));
                errorExplanation.append(tr("Common Name (CN)") + "          " + getSInfo(cert.issuerInfo(QSslCertificate::CommonName)));
                errorExplanation.append(tr("Organization (O)") + "          " + getSInfo(cert.issuerInfo(QSslCertificate::Organization)));
                errorExplanation.append(tr("Organizational Unit (OU)") + "  " + getSInfo(cert.issuerInfo(QSslCertificate::Organization)));
                errorExplanation.append("");
                errorExplanation.append(tr("VALIDITY:"));
                errorExplanation.append(tr("Effective Beginning") + "       " + cert.effectiveDate().toString(Qt::SystemLocaleLongDate));
                errorExplanation.append(tr("Expiring On") + "               " + cert.expiryDate().toString(Qt::SystemLocaleLongDate));
                errorExplanation.append("");
                errorExplanation.append(tr("FINGERPRINTS:"));
                errorExplanation.append(tr("SHA-256") + "                   " + cert.digest(QCryptographicHash::Sha256).toHex());
                errorExplanation.append(tr("SHA-1") + "                     " + cert.digest(QCryptographicHash::Sha1).toHex());

                return errorExplanation.join("\n").toUtf8();
            }
            case QNetworkReply::AuthenticationRequiredError: {
                error = tr("Authentication is required to access this resource. Hit Retry to input your credentials and redo the request.");
                return QByteArray();
            }
            default:
                error = tr("An error occurred trying to retrieve the resource: ") + reply->errorString();
                return QByteArray();
        }
    });
}

void HttpBackend::setRedirect(bool redirect) {
    this->redirect = redirect;
}

QString HttpBackend::documentTitle() {
    return fileUrl.host();
}

QUrl HttpBackend::url() {
    return fileUrl;
}

bool HttpBackend::readOnly() {
    return true;
}
