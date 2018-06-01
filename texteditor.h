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
#include "SourceControl/gitintegration.h"

class TabButton;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

    class TextEditorLeftMargin : public QWidget
    {
    public:
        TextEditorLeftMargin(TextEditor *editor) : QWidget(editor) {
            this->editor = editor;
            this->setCursor(Qt::ArrowCursor);
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
        }

        TextEditor *editor;
    };

public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

    QString filename();
    bool isEdited();
    SyntaxHighlighter* highlighter();

    void leftMarginPaintEvent(QPaintEvent *event);
    int leftMarginWidth();

    GitIntegration* git = nullptr;

signals:
    void fileNameChanged();
    void editedChanged();

public slots:
    TabButton* getTabButton();
    void setActive(bool active);

    void openFile(QString file);
    void openFileFake(QString filename, QString contents);
    bool saveFile(QString file);
    bool saveFile();

    void setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections);

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
    int brokenLine = -1;

    int highlightedLine = -1;
};

#endif // TEXTEDITOR_H
