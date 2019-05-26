#include "plugin.h"

#include <QAction>
#include <QFileDialog>
#include <QEventLoop>
#include "mdpreviewpane.h"

Plugin::Plugin() : AuxiliaryPanePlugin()
{
    Q_INIT_RESOURCE(mdpreview_resources);
}

QList<AuxiliaryPaneCapabilities> Plugin::getPanes() {
    return {
        AuxiliaryPaneCapabilities({
            tr("Markdown Previewer"),
            [=] {
                return new MdPreviewPane();
            }
        })
    };
}

QList<AuxiliaryPaneCapabilities> Plugin::getPanesForUrl(QUrl url) {
    if (url.fileName().endsWith(".md")) {
        return {
            AuxiliaryPaneCapabilities({
                tr("Markdown Previewer"),
                [=] {
                    return new MdPreviewPane();
                }
            })
        };
    } else {
        return QList<AuxiliaryPaneCapabilities>();
    }
}
