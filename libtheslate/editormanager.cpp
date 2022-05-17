#include "editormanager.h"

#include "editors/texteditor/texteditorfactory.h"
#include <QMap>

struct EditorManagerPrivate {
        QMap<QString, AbstractEditorFactory*> factories;
};

EditorManager::~EditorManager() {
    delete d;
}

AbstractEditor* EditorManager::createEditor(QString editorType) {
    return d->factories.value(editorType)->create();
}

QString EditorManager::editorTypeForFileName(QString fileName) {
    return "text";
}

void EditorManager::registerFactory(QString name, AbstractEditorFactory* factory) {
    d->factories.insert(name, factory);
}

EditorManager::EditorManager(QObject* parent) :
    QObject{parent} {
    d = new EditorManagerPrivate();

    this->registerFactory("text", new TextEditorFactory());
}