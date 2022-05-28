/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#ifndef PLUGIN_H
#define PLUGIN_H

#include <theslateplugin.h>

struct PluginPrivate;
class Plugin : public QObject,
               public TheSlatePlugin {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID TheSlatePlugin_iid FILE "plugin.json")
        Q_INTERFACES(TheSlatePlugin)

    public:
        Plugin();
        ~Plugin();

    private:
        PluginPrivate* d;

        // TheSlatePlugin interface
    public:
        void activate();
        void deactivate();
};

#endif // PLUGIN_H
