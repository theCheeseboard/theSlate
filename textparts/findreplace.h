#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QWidget>
#include "texteditor.h"

class TextEditor;

namespace Ui {
    class FindReplace;
}

class FindReplace : public QWidget
{
        Q_OBJECT

    public:
        explicit FindReplace(TextEditor *parent = 0);
        ~FindReplace();

    private slots:
        void on_findBox_textChanged(const QString &arg1);

        void on_findNext_clicked();

        void on_findPrev_clicked();

        void find(QString text, bool backward = false);

    private:
        Ui::FindReplace *ui;

        TextEditor* editor;
};

#endif // FINDREPLACE_H
