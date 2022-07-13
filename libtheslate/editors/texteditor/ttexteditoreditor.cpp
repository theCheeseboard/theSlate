#include "ttexteditoreditor.h"

#include "breakpointrenderstep.h"
#include "lsp/languageserverexception.h"
#include "lsp/languageserverprocess.h"
#include "project.h"
#include "texteditorcompletionwidget.h"
#include <QBoxLayout>
#include <QMouseEvent>
#include <QToolTip>
#include <QUrl>
#include <completetexteditor.h>
#include <texteditor.h>

struct TTextEditorEditorPrivate {
        CompleteTextEditor* editor;

        int lspFileVersion = 0;

        QTimer hoverTrigger;
};

TTextEditorEditor::TTextEditorEditor(QWidget* parent) :
    AbstractEditor{parent} {
    d = new TTextEditorEditorPrivate();
    d->editor = new CompleteTextEditor(this);

    connect(d->editor->editor(), &TextEditor::currentFileChanged, this, &TTextEditorEditor::currentFileChanged);
    connect(d->editor->editor(), &TextEditor::unsavedChangesChanged, this, &TTextEditorEditor::unsavedChangesChanged);
    connect(d->editor->editor(), &TextEditor::textChanged, this, &TTextEditorEditor::textChanged);
    connect(d->editor->editor(), &TextEditor::keyTyped, this, &TTextEditorEditor::editorKeyTyped);
    d->editor->editor()->installEventFilter(this);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->editor);
    this->setLayout(layout);

    d->hoverTrigger.setInterval(1000);
    d->hoverTrigger.setSingleShot(true);
    connect(&d->hoverTrigger, &QTimer::timeout, this, &TTextEditorEditor::showHover);

    d->editor->editor()->pushRenderStep(new BreakpointRenderStep(d->editor->editor()));
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

QCoro::Task<> TTextEditorEditor::showHover() {
    auto lsp = co_await this->languageServer();
    if (!lsp) co_return;

    auto hoverResponse = co_await lsp->hover(d->editor->editor()->currentFile(), d->editor->editor()->hitTest(d->editor->editor()->mapFromGlobal(QCursor::pos())));
    QToolTip::showText(QCursor::pos(), hoverResponse.text, this);
}

void TTextEditorEditor::editorMouseMove(QMouseEvent* event) {
    QToolTip::hideText();
    d->hoverTrigger.stop();
    d->hoverTrigger.start();
}

void TTextEditorEditor::editorLeave(QEvent* event) {
    QToolTip::hideText();
    d->hoverTrigger.stop();
}

QCoro::Task<> TTextEditorEditor::editorKeyTyped(QString keyText) {
    if (d->editor->editor()->numberOfCarets() != 1) co_return;

    auto anchorStart = d->editor->editor()->caretAnchorStart(0);
    auto anchorEnd = d->editor->editor()->caretAnchorEnd(0);
    if (anchorStart != anchorEnd) co_return;

    auto lsp = co_await this->languageServer();
    if (!lsp) co_return;

    if (!lsp->completionTriggerCharacters().contains(keyText.at(0))) co_return;

    auto completionWidget = new TextEditorCompletionWidget(d->editor->editor(), lsp, this);
    completionWidget->setFont(d->editor->editor()->font());
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

        lsp->call("textDocument/codeLens", QJsonObject({
                                               {"textDocument", QJsonObject({{"uri", d->editor->editor()->currentFile().toString()}})}
        }));
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

bool TTextEditorEditor::eventFilter(QObject* watched, QEvent* event) {
    if (watched == d->editor->editor()) {
        if (event->type() == QEvent::MouseMove) {
            this->editorMouseMove(static_cast<QMouseEvent*>(event));
        } else if (event->type() == QEvent::Leave) {
            this->editorLeave(event);
        }
    }
    return false;
}
