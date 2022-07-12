#ifndef THESLATEPLUGIN_H
#define THESLATEPLUGIN_H

#include <plugins/plugininterface.h>
#include "libtheslate_global.h"

class TheSlatePlugin : public PluginInterface {
public:
	TheSlatePlugin() {};
	~TheSlatePlugin() {};
};

#define TheSlatePlugin_iid "com.vicr123.theslate.PluginInterface/2.0"
Q_DECLARE_INTERFACE(TheSlatePlugin, TheSlatePlugin_iid);

#endif // THESLATEPLUGIN_H
