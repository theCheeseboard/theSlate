#include "statemanager.h"

#include "editormanager.h"
#include "project/buildenginemanager.h"

struct StateManagerPrivate {
        EditorManager* editor;
        BuildEngineManager* buildEngine;
};

StateManager::~StateManager() {
    delete d;
}

EditorManager* StateManager::editor() {
    return instance()->d->editor;
}

BuildEngineManager* StateManager::buildEngine() {
    return instance()->d->buildEngine;
}

StateManager* StateManager::instance() {
    static auto* mgr = new StateManager();
    return mgr;
}

StateManager::StateManager(QObject* parent) :
    QObject{parent} {
    d = new StateManagerPrivate();
    d->editor = new EditorManager();
    d->buildEngine = new BuildEngineManager();
}
