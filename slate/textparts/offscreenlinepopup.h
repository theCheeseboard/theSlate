#ifndef OFFSCREENLINEPOPUP_H
#define OFFSCREENLINEPOPUP_H

#include <QWidget>
#include <QTextBlock>

namespace Ui {
    class OffscreenLinePopup;
}

struct OffscreenLinePopupPrivate;
class TextEditor;
class OffscreenLinePopup : public QWidget
{
        Q_OBJECT

    public:
        enum Side {
            Top,
            Bottom
        };

        explicit OffscreenLinePopup(TextEditor* editor);
        ~OffscreenLinePopup();

        void setSide(Side side);
        void setBlocks(QList<QTextBlock> blocks);
        void clearBlocks();

    private:
        Ui::OffscreenLinePopup *ui;
        OffscreenLinePopupPrivate* d;

        void paintEvent(QPaintEvent* e);
};

#endif // OFFSCREENLINEPOPUP_H
