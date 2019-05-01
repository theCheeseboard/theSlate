#ifndef SELECTLISTDIALOG_H
#define SELECTLISTDIALOG_H

#include <QWidget>
#include <QVariant>

namespace Ui {
    class SelectListDialog;
}

struct SelectListItem {
    SelectListItem(QString text);
    SelectListItem(QString text, QVariant data);

    QString text;
    QVariant data;
    QStringList tags;
};

struct SelectListDialogPrivate;
class SelectListDialog : public QWidget
{
        Q_OBJECT

    public:
        explicit SelectListDialog(QWidget *parent = nullptr);
        ~SelectListDialog();

        void setItems(QList<SelectListItem> items);

    public slots:
        void setTitle(QString title);
        void setText(QString text);

    signals:
        void rejected();
        void accepted(QVariant data);

    private slots:
        void on_cancelButton_clicked();

        void on_acceptButton_clicked();

        void on_searchEdit_textChanged(const QString &arg1);

        void on_listOptions_currentRowChanged(int currentRow);

        void on_searchEdit_returnPressed();

    private:
        Ui::SelectListDialog *ui;

        SelectListDialogPrivate* d;
};

#endif // SELECTLISTDIALOG_H
