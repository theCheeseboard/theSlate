#include "careterasecommand.h"

#include "../textcaret.h"
#include "../texteditor.h"
#include "../texteditor_p.h"

CaretEraseCommand::CaretEraseCommand(TextEditor* editor, bool backwardDelete) :
    TextEditorCommand(editor) {
    // TODO: Check if there are anchors set and delete anchored text only only if so
    for (int i = 0; i < editor->d->carets.length(); i++) {
        QPoint caretPos = editor->d->carets.at(i)->linePos();
        // TODO: If there is an anchor set, delete that
        if (caretPos.x() == 0 && caretPos.y() == 0) {
            // Caret is blocked from moving backwards at beginning of document
        } else if (caretPos.x() == 0) {
            // Erase the line
            this->pushEditorCommand({i, "\n", false, !backwardDelete});
        } else {
            this->pushEditorCommand({i, editor->d->lines.at(caretPos.y())->contents.at(caretPos.x() - 1), false, !backwardDelete});
        }
    }
}

CaretEraseCommand::~CaretEraseCommand() {
}

int CaretEraseCommand::id() const {
    return 2;
}
