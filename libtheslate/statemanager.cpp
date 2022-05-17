#include "statemanager.h"

#include "editormanager.h"

struct StateManagerPrivate {
        EditorManager* editor;
};

StateManager::~StateManager() {
    delete d;
}

EditorManager* StateManager::editor() {
    return instance()->d->editor;
}

StateManager* StateManager::instance() {
    static StateManager* mgr = new StateManager();
    return mgr;
}

StateManager::StateManager(QObject* parent) :
    QObject{parent} {
    d = new StateManagerPrivate();
    d->editor = new EditorManager();
}
