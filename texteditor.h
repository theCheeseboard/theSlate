#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include "tabbutton.h"
#include "syntaxhighlighter.h"
#include "exception.h"

class TabButton;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

    class TextEditorLeftMargin : public QWidget
    {
    public:
        TextEditorLeftMargin(TextEditor *editor) : QWidget(editor) {
            this->editor = editor;
            this->setCursor(Qt::PointingHandCursor);
        }

        QSize sizeHint() const override {
            return QSize(editor->leftMarginWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            editor->leftMarginPaintEvent(event);
        }

    private:
        void mousePressEvent(QMouseEvent* event) override {
            int lineNo = editor->cursorForPosition(event->pos()).block().firstLineNumber() + 1;
            if (editor->hasBreakpoint(lineNo)) {
                editor->removeBreakpoint(lineNo);
            } else {
                editor->addBreakpoint(lineNo);
            }
        }

        TextEditor *editor;
    };

    class ExceptionDialog : public QWidget
    {
    public:
        ExceptionDialog(TextEditor *editor) : QWidget(editor) {
            this->editor = editor;
            this->setCursor(Qt::PointingHandCursor);
            this->setVisible(false);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            editor->exceptionPaintEvent(event);
        }

    private:
        TextEditor *editor;
    };

public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

    QString filename();
    bool isEdited();
    SyntaxHighlighter* highlighter();

    void leftMarginPaintEvent(QPaintEvent *event);
    void exceptionPaintEvent(QPaintEvent *event);
    int leftMarginWidth();

    void enableIDEMode();

signals:
    void fileNameChanged();
    void editedChanged();

    void breakpointSet(int lineNumber);
    void breakpointRemoved(int lineNumber);

public slots:
    TabButton* getTabButton();
    void setActive(bool active);

    void openFile(QString file);
    void openFileFake(QString filename, QString contents);
    bool saveFile(QString file);
    bool saveFile();

    void addBreakpoint(int lineNumber);
    void removeBreakpoint(int lineNumber);
    bool hasBreakpoint(int lineNumber);
    QList<QTextBlock> allBreakpoints();
    void setBrokenLine(int lineNumber);
    void clearBrokenLine();

    void setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections);

    void setException(Exception exception);
    void clearException();

private slots:
    void updateLeftMarginAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLeftMarginArea(const QRect &, int);
    void reloadBlockHighlighting();

private:
    TabButton* button;
    bool active;
    bool edited = false;
    bool firstEdit = true;
    QString fn;
    SyntaxHighlighter* hl;

    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);

    TextEditorLeftMargin *leftMargin = NULL;
    ExceptionDialog* exceptionDialog = NULL;
    QList<QTextBlock> breakpoints;
    int brokenLine = -1;
    Exception currentException;
    bool ideMode = false;

    int highlightedLine = -1;
};

#endif // TEXTEDITOR_H
