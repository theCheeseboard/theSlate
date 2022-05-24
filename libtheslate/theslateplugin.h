#ifndef THESLATEPLUGIN_H
#define THESLATEPLUGIN_H

#include <plugins/plugininterface.h>

class TheSlatePlugin : public PluginInterface {
};

#define TheSlatePlugin_iid "com.vicr123.theslate.PluginInterface/2.0"
Q_DECLARE_INTERFACE(TheSlatePlugin, TheSlatePlugin_iid);

#endif // THESLATEPLUGIN_H
