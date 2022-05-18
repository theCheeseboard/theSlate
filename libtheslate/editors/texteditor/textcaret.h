#ifndef TEXTCARET_H
#define TEXTCARET_H

#include <QObject>

struct TextCaretPrivate;
class TextEditor;
class QPainter;
class TextCaret : public QObject {
        Q_OBJECT
    public:
        explicit TextCaret(int line, int pos, TextEditor* parent = nullptr);
        ~TextCaret();

        struct SavedCaret {
                TextEditor* parent;
                int line;
                int pos;
        };

        static TextCaret* fromSavedCaret(SavedCaret caret);
        SavedCaret saveCaret();
        void loadCaret(SavedCaret caret);

        void moveCaret(int line, int pos);
        void moveCaret(QPoint linePos);
        void moveCaretRelative(int lines, int cols);
        void drawCaret(QPainter* painter);

        void setIsPrimary(bool primary);
        bool isPrimary();

        void insertText(QString text);
        void backspace();

        QPoint linePos();

    signals:

    private:
        TextCaretPrivate* d;

        void moveCaret(QRect newPos);

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // TEXTCARET_H
