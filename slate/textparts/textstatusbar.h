#ifndef TEXTSTATUSBAR_H
#define TEXTSTATUSBAR_H

#include <QWidget>

namespace Ui {
    class TextStatusBar;
}

class TextEditor;
struct TextStatusBarPrivate;

class TextStatusBar : public QWidget
{
        Q_OBJECT

    public:
        explicit TextStatusBar(TextEditor *parent = nullptr);
        ~TextStatusBar();

        int lineEndings();

    public slots:
        void setPosition(int line, int col);
        void setSpacing(bool spaces, int number);
        void setLineEndings(int lineEndings);

    private:
        Ui::TextStatusBar *ui;

        TextStatusBarPrivate* d;
};

#endif // TEXTSTATUSBAR_H
