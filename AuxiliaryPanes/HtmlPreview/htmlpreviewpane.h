#ifndef HTMLPREVIEWPANE_H
#define HTMLPREVIEWPANE_H

#include <QWebEngineView>
#include "../../slate/plugins/auxiliarypane.h"

namespace Ui {
    class HtmlPreviewPane;
}

class HtmlPreviewPane : public AuxiliaryPane
{
        Q_OBJECT

    public:
        explicit HtmlPreviewPane(QWidget *parent = nullptr);
        ~HtmlPreviewPane();

        void parseFile(QUrl fileName, QString fileData);
        void cursorChanged(QTextCursor cursor);

    private:
        Ui::HtmlPreviewPane *ui;

        QWebEngineView* view;
};

#endif // HTMLPREVIEWPANE_H
