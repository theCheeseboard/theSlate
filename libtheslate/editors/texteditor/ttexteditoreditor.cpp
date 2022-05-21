#include "ttexteditoreditor.h"

#include <QBoxLayout>
#include <QUrl>
#include <texteditor.h>

struct TTextEditorEditorPrivate {
        TextEditor* editor;
};

TTextEditorEditor::TTextEditorEditor(QWidget* parent) :
    AbstractEditor{parent} {
    d = new TTextEditorEditorPrivate();
    d->editor = new TextEditor(this);

    connect(d->editor, &TextEditor::currentFileChanged, this, &TTextEditorEditor::currentFileChanged);
    connect(d->editor, &TextEditor::unsavedChangesChanged, this, &TTextEditorEditor::unsavedChangesChanged);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->editor);
    this->setLayout(layout);
}

TTextEditorEditor::~TTextEditorEditor() {
    delete d;
}

void TTextEditorEditor::undo() {
    d->editor->undo();
}

void TTextEditorEditor::redo() {
    d->editor->redo();
}

void TTextEditorEditor::setData(QByteArray data) {
    // TODO: Other encodings
    d->editor->setText(QString::fromUtf8(data));
}

QByteArray TTextEditorEditor::data() {
    return d->editor->text().toUtf8();
}

void TTextEditorEditor::setCurrentUrl(QUrl url) {
    d->editor->setCurrentFile(url);
}

QUrl TTextEditorEditor::currentUrl() {
    return d->editor->currentFile();
}

bool TTextEditorEditor::haveUnsavedChanges() {
    return d->editor->haveUnsavedChanges();
}

void TTextEditorEditor::setChangesSaved() {
    d->editor->setChangesSaved();
}

QStringList TTextEditorEditor::nameFilters() {
    return {
        tr("Text File (*.txt)"),
        tr("All Files (*)")};
}

QString TTextEditorEditor::defaultExtension() {
    return ".txt";
}
