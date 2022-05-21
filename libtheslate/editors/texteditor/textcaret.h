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
                int anchor;
        };

        static TextCaret* fromSavedCaret(SavedCaret caret);
        SavedCaret saveCaret();
        void loadCaret(SavedCaret caret);

        void moveCaret(int line, int pos);
        void moveCaret(QPoint linePos);
        void moveCaretRelative(int lines, int cols);
        void moveCaretToStartOfLine();
        void moveCaretToEndOfLine();

        void setAnchor(int line, int pos);
        void setAnchor(QPoint linePos);
        void moveAnchorRelative(int lines, int cols);

        QPoint firstAnchor();
        QPoint lastAnchor();

        void drawCaret(QPainter* painter);

        void setIsPrimary(bool primary);
        bool isPrimary();

        void insertText(QString text);
        void backspace();

        QPoint linePos();

        void discontinueAndDelete();

    signals:
        void discontinued();

    private:
        TextCaretPrivate* d;

        void moveCaret(QRect newPos);

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

typedef QList<TextCaret::SavedCaret> SavedCarets;

#endif // TEXTCARET_H
