#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <QWidget>
#include "plugins/auxiliarypane.h"

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
        void openAuxPane(AuxiliaryPane* pane);

    public slots:
        void showFindReplace();

    private slots:
        void on_auxEditors_tabCloseRequested(int index);

        void updateAuxMenu();

    private:
        Ui::TextWidget *ui;

        TextWidgetPrivate* d;
};

#endif // TEXTWIDGET_H
