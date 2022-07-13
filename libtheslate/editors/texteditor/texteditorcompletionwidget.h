#ifndef TEXTEDITORCOMPLETIONWIDGET_H
#define TEXTEDITORCOMPLETIONWIDGET_H

#include <QCoroTask>
#include <QWidget>

class TextEditor;
class LanguageServerProcess;
struct TextEditorCompletionWidgetPrivate;
class TextEditorCompletionWidget : public QWidget {
        Q_OBJECT
    public:
        explicit TextEditorCompletionWidget(TextEditor* editor, LanguageServerProcess* lsp, QWidget* parent = nullptr);
        ~TextEditorCompletionWidget();

    signals:

    private:
        TextEditorCompletionWidgetPrivate* d;

        QCoro::Task<> updateCompletions();
        void updatePosition();
        void commitCompletion(int index);

        int itemHeight();
        void ensureSelectedVisible();

        bool editorKeyPress(QKeyEvent* event);

        // QWidget interface
    protected:
        void paintEvent(QPaintEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // TEXTEDITORCOMPLETIONWIDGET_H
