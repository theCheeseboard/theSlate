#include "texteditorfactory.h"

#include "ttexteditoreditor.h"

TextEditorFactory::TextEditorFactory(QObject* parent) :
    AbstractEditorFactory{parent} {
}

AbstractEditor* TextEditorFactory::create() {
    return new TTextEditorEditor();
}
