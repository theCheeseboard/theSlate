#include "texteditorcommand.h"

#include "../texteditor.h"
#include "../texteditor_p.h"

struct TextEditorCommandPrivate {
        TextEditor* editor;
        SavedCarets carets;

        QList<TextEditorCommand::EditorCommand> commands;
};

TextEditorCommand::TextEditorCommand(TextEditor* editor) {
    d = new TextEditorCommandPrivate();
    d->editor = editor;
    d->carets = d->editor->d->saveCarets();
}

TextEditorCommand::~TextEditorCommand() {
    delete d;
}

void TextEditorCommand::pushEditorCommand(EditorCommand command) {
    d->commands.append(command);
}

void TextEditorCommand::undo() {
    d->editor->d->loadCarets(d->carets);
    for (auto command = d->commands.crbegin(); command != d->commands.crend(); command++) {
        TextCaret* caret = d->editor->d->carets.at(command->caret);
        if (command->isInsert) {
            for (int i = 0; i < command->text.length(); i++) {
                caret->backspace();
            }
        } else {
            caret->insertText(command->text);
        }
    }
    d->carets = d->editor->d->saveCarets();
}

void TextEditorCommand::redo() {
    d->editor->d->loadCarets(d->carets);
    for (const EditorCommand& command : d->commands) {
        TextCaret* caret = d->editor->d->carets.at(command.caret);
        if (command.isInsert) {
            caret->insertText(command.text);
        } else {
            for (int i = 0; i < command.text.length(); i++) {
                caret->backspace();
            }
        }
    }
    d->carets = d->editor->d->saveCarets();
}

bool TextEditorCommand::mergeWith(const QUndoCommand* other) {
    if (other->id() != this->id()) return false;

    const TextEditorCommand* otherCommand = static_cast<const TextEditorCommand*>(other);
    if (otherCommand->d->carets.length() != this->d->carets.length()) return false;

    SavedCarets carets = d->carets;
    for (const EditorCommand& command : otherCommand->d->commands) {
        carets[command.caret].pos += command.isInsert ? command.text.length() : -command.text.length();
    }
    for (int i = 0; i < this->d->carets.length(); i++) {
        TextCaret::SavedCaret saved = otherCommand->d->carets.at(i);
        TextCaret::SavedCaret thisSaved = carets.at(i);
        if (saved.line != thisSaved.line) return false;
        if (saved.pos != thisSaved.pos) return false;
    }

    this->d->commands.append(otherCommand->d->commands);
    this->d->carets = otherCommand->d->carets;
    return true;
}
