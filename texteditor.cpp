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
