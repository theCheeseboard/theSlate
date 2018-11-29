#ifndef HTTPBASICAUTHDIALOG_H
#define HTTPBASICAUTHDIALOG_H

#include <QDialog>

namespace Ui {
    class HttpBasicAuthDialog;
}

class HttpBasicAuthDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit HttpBasicAuthDialog(QWidget *parent = nullptr);
        ~HttpBasicAuthDialog();

        QString username();
        QString password();
        void setText(QString text);

    private slots:
        void on_okButton_clicked();

        void on_cancelButton_clicked();

    private:
        Ui::HttpBasicAuthDialog *ui;
};

#endif // HTTPBASICAUTHDIALOG_H
