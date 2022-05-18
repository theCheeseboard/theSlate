#ifndef CARETTEXTCOMMAND_H
#define CARETTEXTCOMMAND_H

#include <QUndoCommand>

class TextEditor;
struct CaretTextCommandPrivate;
class CaretTextCommand : public QUndoCommand {
    public:
        explicit CaretTextCommand(TextEditor* editor, QString text);
        ~CaretTextCommand();

    signals:

    private:
        CaretTextCommandPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
        int id() const;
        bool mergeWith(const QUndoCommand* other);
};

#endif // CARETTEXTCOMMAND_H
