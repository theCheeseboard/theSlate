#ifndef TEXTEDITOR_P_H
#define TEXTEDITOR_P_H

#include "textcaret.h"
#include <QScrollBar>
#include <QString>
#include <QUndoStack>

struct TextEditorPrivate {
        struct Line {
                QString contents;
        };

        QList<Line*> lines;
        QScrollBar *vScrollBar, *hScrollBar;
        QList<int> lineTops;

        QList<TextCaret*> carets;
        QUndoStack* undoStack;

        QList<TextCaret::SavedCaret> saveCarets();
        void loadCarets(QList<TextCaret::SavedCaret> carets);
};

#endif // TEXTEDITOR_P_H
