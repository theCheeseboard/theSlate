#include "ttexteditoreditor.h"

#include <QBoxLayout>
#include <QUrl>
#include <completetexteditor.h>
#include <texteditor.h>

struct TTextEditorEditorPrivate {
        CompleteTextEditor* editor;
};

TTextEditorEditor::TTextEditorEditor(QWidget* parent) :
    AbstractEditor{parent} {
    d = new TTextEditorEditorPrivate();
    d->editor = new CompleteTextEditor(this);

    connect(d->editor->editor(), &TextEditor::currentFileChanged, this, &TTextEditorEditor::currentFileChanged);
    connect(d->editor->editor(), &TextEditor::unsavedChangesChanged, this, &TTextEditorEditor::unsavedChangesChanged);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->editor);
    this->setLayout(layout);
}

TTextEditorEditor::~TTextEditorEditor() {
    delete d;
}

void TTextEditorEditor::undo() {
    d->editor->editor()->undo();
}

void TTextEditorEditor::redo() {
    d->editor->editor()->redo();
}

void TTextEditorEditor::setData(QByteArray data) {
    // TODO: Other encodings
    d->editor->editor()->setText(QString::fromUtf8(data));
}

QByteArray TTextEditorEditor::data() {
    return d->editor->editor()->text().toUtf8();
}

void TTextEditorEditor::setCurrentUrl(QUrl url) {
    d->editor->editor()->setCurrentFile(url);
}

QUrl TTextEditorEditor::currentUrl() {
    return d->editor->editor()->currentFile();
}

bool TTextEditorEditor::haveUnsavedChanges() {
    return d->editor->editor()->haveUnsavedChanges();
}

void TTextEditorEditor::setChangesSaved() {
    d->editor->editor()->setChangesSaved();
}

QStringList TTextEditorEditor::nameFilters() {
    return {
        tr("Text File (*.txt)"),
        tr("All Files (*)")};
}

QString TTextEditorEditor::defaultExtension() {
    return ".txt";
}
