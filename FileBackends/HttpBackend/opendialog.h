#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
    class OpenDialog;
}

class OpenDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit OpenDialog(QWidget *parent = nullptr);
        ~OpenDialog();

        QUrl currentUrl();
        bool redirect();

    private slots:
        void on_openButton_clicked();

        void on_cancelButton_clicked();

        void on_urlToOpen_textChanged(const QString &arg1);

        void on_urlToOpen_editingFinished();

    private:
        Ui::OpenDialog *ui;

        QUrl cUrl;
};

#endif // OPENDIALOG_H
