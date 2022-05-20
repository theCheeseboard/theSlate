#ifndef ABSTRACTEDITORCOLORSCHEME_H
#define ABSTRACTEDITORCOLORSCHEME_H

#include <QObject>

class AbstractEditorColorScheme : public QObject {
        Q_OBJECT
    public:
        explicit AbstractEditorColorScheme(QObject* parent = nullptr);

        enum ColorSchemeItem {
            Background = 0,
            Margin,
            NormalText,
            MarginText,
            Breakpoint,
            ActiveLine,
            ActiveLineMarginText,
            HighlightedText,
            HighlightedTextBorder,
        };
        QBrush item(ColorSchemeItem item);

    signals:
};

#endif // ABSTRACTEDITORCOLORSCHEME_H
