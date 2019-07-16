#include "webpage.h"
#include <QDesktopServices>

WebPage::WebPage(QObject *parent) : QWebEnginePage(parent)
{

}

QWebEnginePage* WebPage::createWindow(QWebEnginePage::WebWindowType type) {
    return nullptr;
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    if (type == NavigationTypeTyped || !isMainFrame) {
        return true;
    }

    QDesktopServices::openUrl(url);
    return false;
}
