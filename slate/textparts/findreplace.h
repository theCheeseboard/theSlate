#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QWidget>
#include "texteditor.h"

class TextEditor;

namespace Ui {
    class FindReplace;
}

struct FindReplacePrivate;
class FindReplace : public QWidget
{
        Q_OBJECT

    public:
        explicit FindReplace(QWidget *parent = 0);
        ~FindReplace();

        void setEditor(TextEditor* editor);

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

        void on_replaceSelectedButton_clicked();

        void on_replaceBox_returnPressed();

    private:
        Ui::FindReplace *ui;
        FindReplacePrivate* d;

        QRegularExpression searchExpression(QString text);

        void hideEvent(QHideEvent* event);
        void showEvent(QShowEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // FINDREPLACE_H
