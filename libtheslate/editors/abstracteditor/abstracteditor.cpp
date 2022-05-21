#include "abstracteditor.h"

#include <QUrl>
#include <texteditorcolorscheme.h>

struct AbstractEditorPrivate {
        TextEditorColorScheme* colorScheme;
        QUrl currentUrl;
};

AbstractEditor::AbstractEditor(QWidget* parent) :
    QWidget{parent} {
    d = new AbstractEditorPrivate();
    d->colorScheme = new TextEditorColorScheme(this);
}

AbstractEditor::~AbstractEditor() {
    delete d;
}

void AbstractEditor::setCurrentUrl(QUrl url) {
    d->currentUrl = url;
    emit currentFileChanged(url);
}

QUrl AbstractEditor::currentUrl()
{
    return d->currentUrl;
}

TextEditorColorScheme* AbstractEditor::colorScheme() {
    return d->colorScheme;
}

void AbstractEditor::setColorScheme(QString scheme) {
}
