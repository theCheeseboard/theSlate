#include "texteditor.h"

TextEditor::TextEditor(QWidget *parent) : QTextEdit(parent)
{
    button = new TabButton(this);
    button->setText("New Document");
    this->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    hl = new SyntaxHighlighter(this->document());

    connect(this, &TextEditor::textChanged, [=] {
        edited = true;
        emit editedChanged();
    });
    connect(this, &TextEditor::fileNameChanged, [=] {
        button->setText(QFileInfo(this->filename()).fileName());
    });
}

TextEditor::~TextEditor() {
    if (button != NULL) {
        button->setVisible(false);
    }
}

TabButton* TextEditor::getTabButton() {
    return button;
}

void TextEditor::setActive(bool active) {
    button->setActive(active);
    this->active = active;
}

QString TextEditor::filename() {
    return fn;
}

bool TextEditor::isEdited() {
    return edited;
}

void TextEditor::openFile(QString file) {
    QFile f(file);
    f.open(QFile::ReadOnly);
    this->setPlainText(f.readAll());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();
}

bool TextEditor::saveFile(QString file) {
    QFile f(file);
    f.open(QFile::WriteOnly);
    f.write(this->toPlainText().toUtf8());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();
    return true;
}

bool TextEditor::saveFile() {
    if (this->filename() == "") {
        return false;
    } else {
        return saveFile(this->filename());
    }
}

SyntaxHighlighter* TextEditor::highlighter() {
    return hl;
}

void TextEditor::keyPressEvent(QKeyEvent *event) {
    QTextCursor cursor = this->textCursor();
    bool handle = false;
    if (event->key() == Qt::Key_Tab) {
        cursor.insertText("    ");
        handle = true;
    } else if (event->key() == Qt::Key_Backspace) {
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        QString leftChar = cursor.selectedText();
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString rightChar = cursor.selectedText();
        if ((leftChar == "(" && rightChar == ")") || (leftChar == "[" && rightChar == "]") || (leftChar == "{" && rightChar == "}")) {
            cursor.deleteChar();
            cursor.deletePreviousChar();
            handle = true;
        } else {
            cursor.movePosition(QTextCursor::Left);
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            QString text = cursor.selectedText();
            if (text.endsWith("    ") && text.length() % 4 == 0) {
                cursor = this->textCursor();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                handle = true;
            }
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        int spaces = 0;
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == "{") {
            spaces = 4;
        }

        cursor.select(QTextCursor::LineUnderCursor);
        QString text = cursor.selectedText();
        while (text.startsWith(" ")) {
            spaces++;
            text.remove(0, 1);
        }
        cursor = this->textCursor();
        cursor.insertText("\n");
        for (int i = 0; i < spaces; i++) {
            cursor.insertText(" ");
        }

        if (cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor)) {
            if (cursor.selectedText() == "}") {
                cursor.movePosition(QTextCursor::Left);
                cursor.insertText("\n");
                for (int i = 0; i < spaces - 4; i++) {
                    cursor.insertText(" ");
                    cursor.movePosition(QTextCursor::Left);
                }
                cursor.movePosition(QTextCursor::Left);
            } else {
                cursor.movePosition(QTextCursor::Left);
            }
        }

        handle = true;
    } else if (event->text() == "}") {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString text = cursor.selectedText();
        if (text.endsWith("    ") && text.length() % 4 == 0) {
            cursor = this->textCursor();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
            cursor.insertText("}");
            handle = true;
        }
    } else if (event->text() == "{") {
        cursor.insertText("{");
        cursor.insertText("}");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == "[") {
        cursor.insertText("[");
        cursor.insertText("]");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == "(") {
        cursor.insertText("(");
        cursor.insertText(")");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == ")") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == ")") {
            cursor = this->textCursor();
            cursor.movePosition(QTextCursor::Right);
            handle = true;
        }
    } else if (event->text() == "]") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == "]") {
            cursor = this->textCursor();
            cursor.movePosition(QTextCursor::Right);
            handle = true;
        }
    } else if (event->text() == "\"") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString right = cursor.selectedText();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
        QString left = cursor.selectedText();
        if (left != "\\") {
            if (right == "\"") {
                cursor = this->textCursor();
                cursor.movePosition(QTextCursor::Right);
            } else {
                cursor = this->textCursor();
                cursor.insertText("\"");
                cursor.insertText("\"");
                cursor.movePosition(QTextCursor::Left);
            }
            handle = true;
        }
    } else if (event->text() == "'") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString right = cursor.selectedText();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
        QString left = cursor.selectedText();
        if (left != "\\") {
            if (right == "'") {
                cursor = this->textCursor();
                cursor.movePosition(QTextCursor::Right);
            } else {
                cursor = this->textCursor();
                cursor.insertText("'");
                cursor.insertText("'");
                cursor.movePosition(QTextCursor::Left);
            }
            handle = true;
        }
    }

    if (handle) {
        this->setTextCursor(cursor);
    } else {
        QTextEdit::keyPressEvent(event);
    }
}
