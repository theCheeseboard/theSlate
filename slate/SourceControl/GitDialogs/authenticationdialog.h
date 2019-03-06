#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H

#include <QDialog>

namespace Ui {
    class AuthenticationDialog;
}

class AuthenticationDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit AuthenticationDialog(QWidget *parent = nullptr);
        ~AuthenticationDialog();

        void setMessage(QString message);
        QString username();
        QString password();

    private:
        Ui::AuthenticationDialog *ui;
};

#endif // AUTHENTICATIONDIALOG_H
