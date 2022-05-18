#include "carettextcommand.h"

#include "../textcaret.h"
#include "../texteditor.h"
#include "../texteditor_p.h"

struct CaretTextCommandPrivate {
        TextEditor* editor;
        QString text;

        SavedCarets carets;
};

CaretTextCommand::CaretTextCommand(TextEditor* editor, QString text) :
    QUndoCommand{} {
    d = new CaretTextCommandPrivate();
    d->editor = editor;
    d->text = text;

    d->carets = d->editor->d->saveCarets();
}

CaretTextCommand::~CaretTextCommand() {
    delete d;
}

void CaretTextCommand::undo() {
    d->editor->d->loadCarets(d->carets);
    for (auto caret = d->editor->d->carets.rbegin(); caret != d->editor->d->carets.rend(); caret++) {
        for (int i = 0; i < d->text.length(); i++) {
            (*caret)->backspace();
        }
    }
    d->carets = d->editor->d->saveCarets();
}

void CaretTextCommand::redo() {
    d->editor->d->loadCarets(d->carets);
    for (TextCaret* caret : d->editor->d->carets) {
        caret->insertText(d->text);
    }
    d->carets = d->editor->d->saveCarets();
}

int CaretTextCommand::id() const {
    return 1;
}

bool CaretTextCommand::mergeWith(const QUndoCommand* other) {
    if (other->id() != this->id()) return false;

    const CaretTextCommand* otherCommand = static_cast<const CaretTextCommand*>(other);
    if (otherCommand->d->carets.length() != this->d->carets.length()) return false;
    for (int i = 0; i < this->d->carets.length(); i++) {
        TextCaret::SavedCaret saved = otherCommand->d->carets.at(i);
        TextCaret::SavedCaret thisSaved = this->d->carets.at(i);
        if (saved.line != thisSaved.line) return false;
        if (saved.pos != thisSaved.pos + otherCommand->d->text.length()) return false;
    }

    this->d->text += otherCommand->d->text;
    this->d->carets = otherCommand->d->carets;
    return true;
}
