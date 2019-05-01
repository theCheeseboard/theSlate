#ifndef TEXTSTATUSBAR_H
#define TEXTSTATUSBAR_H

#include <QWidget>
#include <Definition>

namespace Ui {
    class TextStatusBar;
}

class TextEditor;
struct TextStatusBarPrivate;

class TextStatusBar : public QWidget
{
        Q_OBJECT

    public:
        explicit TextStatusBar(QWidget *parent = nullptr);
        ~TextStatusBar();

        int lineEndings();

    public slots:
        void setEditor(TextEditor* editor);
        void setPosition(int line, int col);
        void setSpacing(bool spaces, int number);
        void setLineEndings(int lineEndings);
        void setHighlighting(KSyntaxHighlighting::Definition definition);
        void setEncoding(QString encodingName);

    private slots:
        void on_highlightingButton_clicked();

        void on_encodingButton_clicked();

    private:
        Ui::TextStatusBar *ui;

        TextStatusBarPrivate* d;
};

#endif // TEXTSTATUSBAR_H
