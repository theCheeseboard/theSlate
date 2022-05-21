#ifndef CARETTEXTCOMMAND_H
#define CARETTEXTCOMMAND_H

#include "texteditorcommand.h"

class TextEditor;
class CaretTextCommand : public TextEditorCommand {
    public:
        explicit CaretTextCommand(TextEditor* editor, QString text);
        ~CaretTextCommand();

    signals:

        // QUndoCommand interface
    public:
        int id() const;
};

#endif // CARETTEXTCOMMAND_H
