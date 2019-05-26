#ifndef PLUGIN_H
#define PLUGIN_H

#include "../../slate/plugins/auxiliarypane.h"

class Plugin : public QObject, public AuxiliaryPanePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID THESLATE_AUXILIARYPANE_IID FILE "MarkdownPreview.json")
    Q_INTERFACES(AuxiliaryPanePlugin)

    public:
        Plugin();

        QList<AuxiliaryPaneCapabilities> getPanes();
        QList<AuxiliaryPaneCapabilities> getPanesForUrl(QUrl url);
};

#endif // PLUGIN_H
