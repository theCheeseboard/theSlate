#ifndef TEXTEDITORCOMMAND_H
#define TEXTEDITORCOMMAND_H

#include <QUndoCommand>

class TextEditor;
struct TextEditorCommandPrivate;
class TextEditorCommand : public QUndoCommand {
    public:
        TextEditorCommand(TextEditor* editor);
        ~TextEditorCommand();

        struct EditorCommand {
                int caret;
                QString text;
                bool isInsert;
                bool insertAfter;
        };

        void pushEditorCommand(EditorCommand command);

    private:
        TextEditorCommandPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
        bool mergeWith(const QUndoCommand* other);
};

#endif // TEXTEDITORCOMMAND_H
