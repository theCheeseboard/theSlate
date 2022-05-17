#include "abstracteditorcolorscheme.h"

#include <QBrush>

AbstractEditorColorScheme::AbstractEditorColorScheme(QObject* parent) :
    QObject{parent} {
}

QBrush AbstractEditorColorScheme::item(ColorSchemeItem item) {
    switch (item) {
        case AbstractEditorColorScheme::Background:
            return QColor(40, 40, 40);
        case AbstractEditorColorScheme::Margin:
            return QColor(40, 40, 40);
        case AbstractEditorColorScheme::NormalText:
            return QColor(255, 255, 255);
        case AbstractEditorColorScheme::MarginText:
            return QColor(200, 200, 200);
        case AbstractEditorColorScheme::Breakpoint:
            return QColor(50, 170, 80);
        case AbstractEditorColorScheme::ActiveLine:
            return QColor(100, 100, 100, 100);
        case AbstractEditorColorScheme::ActiveLineMarginText:
            return QColor(255, 255, 255);
    }

    return QColor();
}
