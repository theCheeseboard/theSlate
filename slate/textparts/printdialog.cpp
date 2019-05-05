#include "printdialog.h"
#include "ui_printdialog.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include "texteditor.h"

PrintDialog::PrintDialog(TextEditor* editor, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);

    #ifdef Q_OS_MAC
        ui->cancelButton->setVisible(false);
        ui->acceptButton->setVisible(false);
    #else
        ui->windowControlsMac->setVisible(false);
    #endif

    this->editor = editor;
    printer = new QPrinter(QPrinterInfo::defaultPrinter(), QPrinter::PrinterResolution);

    previewWidget = new QPrintPreviewWidget(printer);
    previewWidget->setZoomMode(QPrintPreviewWidget::FitToWidth);
    connect(previewWidget, &QPrintPreviewWidget::paintRequested, [=](QPrinter* printer) {
        editor->print(printer);
        ui->acceptButton->setEnabled(this->printer->isValid());
    });
    ui->printPreviewWidget->layout()->addWidget(previewWidget);

    ui->printerSelection->addItems(QPrinterInfo::availablePrinterNames());
    setPrinter(QPrinterInfo::defaultPrinterName());
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

void PrintDialog::setPrinter(QString printerName) {
    printer->setPrinterName(printerName);
    ui->printerSelection->setCurrentText(printerName);
    previewWidget->updatePreview();
}

void PrintDialog::on_printerSelection_currentIndexChanged(const QString &arg1)
{
    setPrinter(arg1);
}

void PrintDialog::on_printPages_toggled(bool checked)
{
    if (checked) {
        printer->setPrintRange(QPrinter::PageRange);
        ui->printPagesFrom->setEnabled(true);
        ui->printPagesTo->setEnabled(true);
        previewWidget->updatePreview();
    } else {
        ui->printPagesFrom->setEnabled(false);
        ui->printPagesTo->setEnabled(false);
    }
}

void PrintDialog::on_printSelection_toggled(bool checked)
{
    if (checked) {
        printer->setPrintRange(QPrinter::Selection);
        previewWidget->updatePreview();
    }
}

void PrintDialog::on_printAll_toggled(bool checked)
{
    if (checked) {
        printer->setPrintRange(QPrinter::AllPages);
        previewWidget->updatePreview();
    }
}

void PrintDialog::on_printPagesFrom_valueChanged(int arg1)
{
    printer->setFromTo(arg1, ui->printPagesTo->value());
    previewWidget->updatePreview();
}

void PrintDialog::on_printPagesTo_valueChanged(int arg1)
{
    printer->setFromTo(ui->printPagesFrom->value(), arg1);
    previewWidget->updatePreview();
}

void PrintDialog::on_spinBox_valueChanged(int arg1)
{
    printer->setCopyCount(arg1);
}

void PrintDialog::on_cancelButton_clicked()
{
    this->close();
}

void PrintDialog::on_portraitButton_toggled(bool checked)
{
    if (checked) {
        printer->setOrientation(QPrinter::Portrait);
        if (ui->duplexBox->isChecked()) printer->setDuplex(QPrinter::DuplexLongSide);
        previewWidget->updatePreview();
    }
}

void PrintDialog::on_landscapeButton_toggled(bool checked)
{
    if (checked) {
        printer->setOrientation(QPrinter::Landscape);
        if (ui->duplexBox->isChecked()) printer->setDuplex(QPrinter::DuplexShortSide);
        previewWidget->updatePreview();
    }
}

void PrintDialog::on_duplexBox_toggled(bool checked)
{
    if (checked) {
        if (ui->portraitButton->isChecked()) {
            printer->setDuplex(QPrinter::DuplexLongSide);
        } else {
            printer->setDuplex(QPrinter::DuplexShortSide);
        }
    } else {
        printer->setDuplex(QPrinter::DuplexNone);
    }

}

void PrintDialog::on_acceptButton_clicked()
{
    editor->print(printer);
    this->close();
}
