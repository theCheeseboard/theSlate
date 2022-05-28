#include "statemanager.h"

#include "editormanager.h"
#include "project/buildenginemanager.h"
#include "project/logscannermanager.h"

struct StateManagerPrivate {
        EditorManager* editor;
        BuildEngineManager* buildEngine;
        LogScannerManager* logScanner;
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

LogScannerManager* StateManager::logScanner() {
    return instance()->d->logScanner;
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
    d->logScanner = new LogScannerManager();
}
