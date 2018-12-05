#include "messagebox.h"

MessageBox::MessageBox(QWidget *parent) : QMessageBox(parent)
{

}

void MessageBox::setWindowTitle(QString windowTitle) {
#ifdef Q_OS_MAC
    QMessageBox::setText(windowTitle);
#else
    QMessageBox::setWindowTitle(windowTitle);
#endif
}

void MessageBox::setText(QString text) {
#ifdef Q_OS_MAC
    QMessageBox::setInformativeText(text);
#else
    QMessageBox::setText(text);
#endif
}
