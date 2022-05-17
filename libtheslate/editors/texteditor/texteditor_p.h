#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include <QScrollBar>
#include <QString>

class TextCaret;
struct TextEditorPrivate {
        struct Line {
                QString contents;
        };

        QList<Line*> lines;
        QScrollBar *vScrollBar, *hScrollBar;
        QList<int> lineTops;

        QList<TextCaret*> carets;
};

#endif // TEXTEDITOR_P_H
