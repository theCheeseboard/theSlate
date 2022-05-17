#include "texteditorfactory.h"

#include "texteditor.h"

TextEditorFactory::TextEditorFactory(QObject* parent) :
    AbstractEditorFactory{parent} {
}

AbstractEditor* TextEditorFactory::create() {
    return new TextEditor();
}
