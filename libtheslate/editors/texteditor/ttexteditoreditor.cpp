#include "ttexteditoreditor.h"

#include "lsp/languageserverexception.h"
#include "lsp/languageserverprocess.h"
#include "project.h"
#include <QBoxLayout>
#include <QUrl>
#include <completetexteditor.h>
#include <texteditor.h>

struct TTextEditorEditorPrivate {
        CompleteTextEditor* editor;

        int lspFileVersion = 0;
};

TTextEditorEditor::TTextEditorEditor(QWidget* parent) :
    AbstractEditor{parent} {
    d = new TTextEditorEditorPrivate();
    d->editor = new CompleteTextEditor(this);

    connect(d->editor->editor(), &TextEditor::currentFileChanged, this, &TTextEditorEditor::currentFileChanged);
    connect(d->editor->editor(), &TextEditor::unsavedChangesChanged, this, &TTextEditorEditor::unsavedChangesChanged);
    connect(d->editor->editor(), &TextEditor::textChanged, this, &TTextEditorEditor::textChanged);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->editor);
    this->setLayout(layout);
}

TTextEditorEditor::~TTextEditorEditor() {
    QUrl oldUrl = d->editor->editor()->currentFile();
    if (oldUrl.isValid()) {
        this->languageServer().then([oldUrl](LanguageServerProcess* lsp) {
            if (lsp && oldUrl.isValid()) lsp->didClose(oldUrl);
        });
    }
    delete d;
}

QCoro::Task<LanguageServerProcess*> TTextEditorEditor::languageServer() {
    if (!this->project()) co_return nullptr;
    co_return co_await this->project()->languageServerForFileName(d->editor->editor()->currentFile().toLocalFile());
}

QCoro::Task<> TTextEditorEditor::textChanged(QList<TextDelta> deltas) {
    auto lsp = co_await this->languageServer();
    if (!lsp) co_return;

    if (lsp->prefersIncrementalSync()) {
        QList<LanguageServerProcess::TextDocumentContentChangeEvent> events;
        for (auto delta : deltas) {
            auto event = LanguageServerProcess::TextDocumentContentChangeEvent();
            event.text = delta.replacement;
            event.start = delta.startEdit;
            event.end = delta.endEdit;
            events.append(event);
        }
        lsp->didChange(this->currentUrl(), d->lspFileVersion, events);
    } else {
        lsp->didChange(this->currentUrl(), d->lspFileVersion, d->editor->editor()->text());
    }
    d->lspFileVersion++;
}

QCoro::Task<> TTextEditorEditor::updateDiagnostics() {
    d->editor->editor()->clearLineProperties(TextEditor::CompilationError);
    d->editor->editor()->clearLineProperties(TextEditor::CompilationWarning);

    auto lsp = co_await this->languageServer();
    if (!lsp) co_return;

    for (auto dg : lsp->diagnostics(d->editor->editor()->currentFile())) {
        TextEditor::KnownLineProperty lineProperty;
        switch (dg.severity) {
            case LanguageServerProcess::Diagnostic::Severity::Error:
                lineProperty = TextEditor::CompilationError;
                break;
            case LanguageServerProcess::Diagnostic::Severity::Warning:
            case LanguageServerProcess::Diagnostic::Severity::Information:
            case LanguageServerProcess::Diagnostic::Severity::Hint:
                lineProperty = TextEditor::CompilationWarning;
                break;
        }

        d->editor->editor()->setLineProperty(dg.start.y(), lineProperty, dg.message);
    }
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
    QUrl oldUrl = d->editor->editor()->currentFile();
    this->languageServer().then([this, oldUrl](LanguageServerProcess* lsp) {
        if (!lsp) return;
        if (oldUrl.isValid()) lsp->didClose(oldUrl);
        lsp->disconnect(this);
    });

    d->editor->editor()->setCurrentFile(url);

    this->languageServer().then([this, url](LanguageServerProcess* lsp) {
        if (!lsp) return;
        if (url.isValid()) lsp->didOpen(url, "cpp" /* TODO? */, d->lspFileVersion, d->editor->editor()->text());

        connect(lsp, &LanguageServerProcess::publishDiagnostics, this, [this, url](QUrl dgUrl) {
            if (dgUrl == url) this->updateDiagnostics();
        });
        this->updateDiagnostics();
    });
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
