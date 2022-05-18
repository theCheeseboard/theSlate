#include "careterasecommand.h"

#include "../textcaret.h"
#include "../texteditor.h"
#include "../texteditor_p.h"

struct CaretEraseCommandPrivate {
        TextEditor* editor;
        SavedCarets carets;

        struct EraseCommand {
                int caret;
                QString savedText;
        };
        QList<EraseCommand> eraseCommands;

        bool backwardDelete;
};

CaretEraseCommand::CaretEraseCommand(TextEditor* editor, bool backwardDelete) {
    d = new CaretEraseCommandPrivate();
    d->editor = editor;
    d->carets = editor->d->saveCarets();
    d->backwardDelete = backwardDelete;

    // TODO: Check if there are anchors set and delete anchored text only only if so
    for (int i = 0; i < editor->d->carets.length(); i++) {
        QPoint caretPos = editor->d->carets.at(i)->linePos();
        // TODO: If there is an anchor set, delete that
        if (backwardDelete) {
            if (caretPos.x() == 0 && caretPos.y() == 0) {
                // Caret is blocked from moving backwards at beginning of document
                d->eraseCommands.append({i, ""});
            } else if (caretPos.x() == 0) {
                // Erase the line
                d->eraseCommands.append({i, "\n"});
            } else {
                d->eraseCommands.append({i, d->editor->d->lines.at(caretPos.y())->contents.at(caretPos.x() - 1)});
            }
        } else {
            // TODO
        }
    }
}

CaretEraseCommand::~CaretEraseCommand() {
    delete d;
}

void CaretEraseCommand::undo() {
    d->editor->d->loadCarets(d->carets);
    for (auto command = d->eraseCommands.rbegin(); command != d->eraseCommands.rend(); command++) {
        TextCaret* caret = d->editor->d->carets.at(command->caret);
        for (int i = 0; i < command->savedText.length(); i++) {
            if (d->backwardDelete) {
                caret->insertText(command->savedText);
            } else {
                // TODO
            }
        }
    }
    d->carets = d->editor->d->saveCarets();
}

void CaretEraseCommand::redo() {
    d->editor->d->loadCarets(d->carets);
    for (CaretEraseCommandPrivate::EraseCommand command : d->eraseCommands) {
        TextCaret* caret = d->editor->d->carets.at(command.caret);
        for (int i = 0; i < command.savedText.length(); i++) {
            if (d->backwardDelete) {
                caret->backspace();
            } else {
                // TODO
            }
        }
    }
    d->carets = d->editor->d->saveCarets();
}

int CaretEraseCommand::id() const {
    return 2;
}

bool CaretEraseCommand::mergeWith(const QUndoCommand* other) {
    if (other->id() != this->id()) return false;

    const CaretEraseCommand* otherCommand = static_cast<const CaretEraseCommand*>(other);
    if (otherCommand->d->carets.length() != this->d->carets.length()) return false;
    if (otherCommand->d->backwardDelete != this->d->backwardDelete) return false;
    for (int i = 0; i < this->d->carets.length(); i++) {
        TextCaret::SavedCaret saved = otherCommand->d->carets.at(i);
        TextCaret::SavedCaret thisSaved = this->d->carets.at(i);
        if (saved.line != thisSaved.line) return false;
        if (d->backwardDelete) {
            if (saved.pos != thisSaved.pos - otherCommand->d->eraseCommands.at(i).savedText.length()) return false;
        } else {
        }
    }

    this->d->eraseCommands.append(otherCommand->d->eraseCommands);
    this->d->carets = otherCommand->d->carets;
    return true;
}
