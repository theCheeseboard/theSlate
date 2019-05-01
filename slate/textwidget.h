#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <QWidget>

namespace Ui {
    class TextWidget;
}

class TextEditor;
class MainWindow;
struct TextWidgetPrivate;
class TextWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit TextWidget(MainWindow *parent);
        ~TextWidget();

        TextEditor* editor();

    public slots:
        void showFindReplace();

    private:
        Ui::TextWidget *ui;

        TextWidgetPrivate* d;
};

#endif // TEXTWIDGET_H
