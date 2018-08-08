#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QWidget>
#include <QPrinter>
#include <QPrintPreviewWidget>
#include <QPrinterInfo>
#include "texteditor.h"

namespace Ui {
    class PrintDialog;
}

class PrintDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit PrintDialog(TextEditor* editor, QWidget *parent = nullptr);
        ~PrintDialog();

    public slots:
        void setPrinter(QString printerName);

    private slots:
        void on_printerSelection_currentIndexChanged(const QString &arg1);

        void on_printPages_toggled(bool checked);

        void on_printSelection_toggled(bool checked);

        void on_printAll_toggled(bool checked);

        void on_printPagesFrom_valueChanged(int arg1);

        void on_printPagesTo_valueChanged(int arg1);

        void on_spinBox_valueChanged(int arg1);

        void on_cancelButton_clicked();

        void on_portraitButton_toggled(bool checked);

        void on_landscapeButton_toggled(bool checked);

        void on_duplexBox_toggled(bool checked);

        void on_acceptButton_clicked();

    private:
        Ui::PrintDialog *ui;

        QPrinter* printer;
        TextEditor* editor;
        QPrintPreviewWidget* previewWidget;
};

#endif // PRINTDIALOG_H
