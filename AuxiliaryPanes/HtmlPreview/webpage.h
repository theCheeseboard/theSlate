#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QObject>
#include <QWebEnginePage>

class WebPage : public QWebEnginePage
{
        Q_OBJECT
    public:
        explicit WebPage(QObject *parent = nullptr);

        bool acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
        QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) override;
    signals:

    public slots:

};

#endif // WEBPAGE_H
