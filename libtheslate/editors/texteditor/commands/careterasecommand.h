#ifndef CARETERASECOMMAND_H
#define CARETERASECOMMAND_H

#include <QUndoCommand>

class TextEditor;
struct CaretEraseCommandPrivate;
class CaretEraseCommand : public QUndoCommand {
    public:
        CaretEraseCommand(TextEditor* editor, bool backwardDelete);
        ~CaretEraseCommand();

    private:
        CaretEraseCommandPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
        int id() const;
        bool mergeWith(const QUndoCommand* other);
};

#endif // CARETERASECOMMAND_H
