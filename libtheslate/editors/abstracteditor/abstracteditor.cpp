#include "abstracteditor.h"

#include <QUrl>
#include <texteditorcolorscheme.h>

struct AbstractEditorPrivate {
        TextEditorColorScheme* colorScheme;
        QUrl currentUrl;

        ProjectPtr project;
};

AbstractEditor::AbstractEditor(QWidget* parent) :
    QWidget{parent} {
    d = new AbstractEditorPrivate();
    d->colorScheme = new TextEditorColorScheme(this);
}

AbstractEditor::~AbstractEditor() {
    delete d;
}

void AbstractEditor::setProject(ProjectPtr project) {
    d->project = project;
    emit projectChanged(project);
}

ProjectPtr AbstractEditor::project() {
    return d->project;
}

void AbstractEditor::setCurrentUrl(QUrl url) {
    d->currentUrl = url;
    emit currentFileChanged(url);
}

QUrl AbstractEditor::currentUrl() {
    return d->currentUrl;
}

TextEditorColorScheme* AbstractEditor::colorScheme() {
    return d->colorScheme;
}

void AbstractEditor::setColorScheme(QString scheme) {
}
