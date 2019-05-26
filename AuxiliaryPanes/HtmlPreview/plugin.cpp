#include "plugin.h"

#include <QAction>
#include <QFileDialog>
#include <QEventLoop>
#include "htmlpreviewpane.h"

Plugin::Plugin() : AuxiliaryPanePlugin()
{

}

QList<AuxiliaryPaneCapabilities> Plugin::getPanes() {
    return {
        AuxiliaryPaneCapabilities({
            tr("HTML Previewer"),
            [=] {
                return new HtmlPreviewPane();
            }
        })
    };
}

QList<AuxiliaryPaneCapabilities> Plugin::getPanesForUrl(QUrl url) {
    if (url.fileName().endsWith(".html") || url.fileName().endsWith(".htm")) {
        return {
            AuxiliaryPaneCapabilities({
                tr("HTML Previewer"),
                [=] {
                    return new HtmlPreviewPane();
                }
            })
        };
    } else {
        return QList<AuxiliaryPaneCapabilities>();
    }
}
