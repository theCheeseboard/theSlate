#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
    class ProgressDialog;
}

class ProgressDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ProgressDialog(QWidget *parent = nullptr);
        ~ProgressDialog();

        void setTitle(QString title);
        void setMessage(QString message);
        void setCancelable(bool cancelable);

    signals:
        void cancel();

    private slots:
        void on_cancelButton_clicked();

    private:
        Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
