#include "abstractpage.h"

AbstractPage::AbstractPage(QWidget* parent) :
    QWidget{parent} {
}

QCoro::Task<> AbstractPage::saveAndClose(bool silent) {
    try {
        co_await this->saveBeforeClose(silent);
        emit done();
    } catch (QException& ex) {
        // Ignore
    }
}
