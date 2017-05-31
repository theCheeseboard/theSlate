#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include "tabbutton.h"
#include "syntaxhighlighter.h"

class TabButton;

class TextEditor : public QTextEdit
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
    QString fn;
    SyntaxHighlighter* hl;
};

#endif // TEXTEDITOR_H
