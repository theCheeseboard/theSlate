#include "abstracteditor.h"

#include "abstracteditorcolorscheme.h"

struct AbstractEditorPrivate {
        AbstractEditorColorScheme* colorScheme;
};

AbstractEditor::AbstractEditor(QWidget* parent) :
    QWidget{parent} {
    d = new AbstractEditorPrivate();
    d->colorScheme = new AbstractEditorColorScheme(this);
}

AbstractEditor::~AbstractEditor() {
    delete d;
}

AbstractEditorColorScheme* AbstractEditor::colorScheme() {
    return d->colorScheme;
}

void AbstractEditor::setColorScheme(QString scheme) {
}
