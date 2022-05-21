#ifndef CARETERASECOMMAND_H
#define CARETERASECOMMAND_H

#include "texteditorcommand.h"

class TextEditor;
class CaretEraseCommand : public TextEditorCommand {
    public:
        CaretEraseCommand(TextEditor* editor, bool backwardDelete);
        ~CaretEraseCommand();

    private:
        // QUndoCommand interface
    public:
        int id() const;
};

#endif // CARETERASECOMMAND_H
