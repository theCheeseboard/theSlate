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
    return {
        AuxiliaryPaneCapabilities({
            tr("HTML Previewer"),
            [=] {
                return new HtmlPreviewPane();
            }
        })
    };
}
