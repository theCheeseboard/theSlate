#include "texteditorblockdata.h"
#include "texteditor.h"

TextEditorBlockData::TextEditorBlockData(TextEditor* parent) {
    editedConnection = QObject::connect(parent, &TextEditor::editedChanged, [=] {
        if (!parent->isEdited() && this->marginState == Edited) this->marginState = SavedEdited;
    });
}

TextEditorBlockData::~TextEditorBlockData() {
    QObject::disconnect(editedConnection);
}
