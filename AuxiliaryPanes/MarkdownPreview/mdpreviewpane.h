#ifndef MDPREVIEWPANE_H
#define MDPREVIEWPANE_H

#include <QWidget>
#include <QJSEngine>
#include <QJSValue>
#include <QWebEngineView>
#include "../../slate/plugins/auxiliarypane.h"

namespace Ui {
    class MdPreviewPane;
}

class MdPreviewPane : public AuxiliaryPane
{
        Q_OBJECT

    public:
        explicit MdPreviewPane(QWidget *parent = nullptr);
        ~MdPreviewPane();

        void parseFile(QUrl fileName, QString fileData);
        void cursorChanged(QTextCursor cursor);

    private slots:

    private:
        Ui::MdPreviewPane *ui;

        QJSEngine js;
        QWebEngineView* view;
};

#endif // MDPREVIEWPANE_H
