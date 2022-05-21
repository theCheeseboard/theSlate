#include "abstractpage.h"

AbstractPage::AbstractPage(QWidget* parent) :
    QWidget{parent} {
}

void AbstractPage::saveAndClose(bool silent) {
    this->saveBeforeClose(silent)->then([=] {
        emit done();
    });
}
