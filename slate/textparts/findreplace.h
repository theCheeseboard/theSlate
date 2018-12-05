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

    public slots:
        void setFocus();

    private slots:
        void on_findBox_textChanged(const QString &arg1);

        void on_findNext_clicked();

        void on_findPrev_clicked();

        void find(QString text);

        void moveCursor(bool backward = false);

        void reset();

        void on_doneButton_clicked();

        void on_replaceAllButton_clicked();

    private:
        Ui::FindReplace *ui;

        TextEditor* editor;
        QList<int> indices;
        int textLength;

        void hideEvent(QHideEvent* event);
        void showEvent(QShowEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // FINDREPLACE_H
