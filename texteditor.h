#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include "tabbutton.h"
#include "syntaxhighlighter.h"

class TabButton;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

    QString filename();
    bool isEdited();
    SyntaxHighlighter* highlighter();
signals:
    void fileNameChanged();
    void editedChanged();

public slots:
    TabButton* getTabButton();
    void setActive(bool active);

    void openFile(QString file);
    bool saveFile(QString file);
    bool saveFile();

private:
    TabButton* button;
    bool active;
    bool edited = false;
    bool firstEdit = true;
    QString fn;
    SyntaxHighlighter* hl;

    void keyPressEvent(QKeyEvent* event);
};

#endif // TEXTEDITOR_H
