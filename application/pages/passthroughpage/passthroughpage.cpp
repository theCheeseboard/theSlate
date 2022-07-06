#include "passthroughpage.h"

#include <twindowtabberbutton.h>

struct PassthroughPagePrivate {
        tWindowTabberButton *tabButton, *defaultTabButton;
        AbstractPage* passthrough = nullptr;
};

PassthroughPage::PassthroughPage(QWidget* parent) :
    AbstractPage{parent} {
    d = new PassthroughPagePrivate();

    d->tabButton = new tWindowTabberButton();
    d->defaultTabButton = new tWindowTabberButton();
    connect(d->defaultTabButton, &tWindowTabberButton::changed, this, &PassthroughPage::copyCurrentTabButton);
}

PassthroughPage::~PassthroughPage() {
    d->defaultTabButton->deleteLater();
    delete d;
}

void PassthroughPage::setPassthroughPage(AbstractPage* page) {
    if (d->passthrough) {
        d->passthrough->tabButton()->disconnect(this);
        d->passthrough->disconnect(this);
    }
    d->passthrough = page;
    if (d->passthrough) {
        // TODO: connect any signals
        connect(d->passthrough->tabButton(), &tWindowTabberButton::changed, this, &PassthroughPage::copyCurrentTabButton);
    }

    copyCurrentTabButton();
}

tWindowTabberButton* PassthroughPage::defaultTabButton() {
    return d->defaultTabButton;
}

void PassthroughPage::copyCurrentTabButton() {
    auto* copyFrom = d->defaultTabButton;
    if (d->passthrough) copyFrom = d->passthrough->tabButton();

    while (!d->tabButton->actions().isEmpty()) d->tabButton->removeAction(d->tabButton->actions().first());
    d->tabButton->setText(copyFrom->text());
    d->tabButton->setIcon(copyFrom->icon());
    d->tabButton->addActions(copyFrom->actions());
}

tWindowTabberButton* PassthroughPage::tabButton() {
    return d->tabButton;
}

void PassthroughPage::undo() {
    if (d->passthrough) d->passthrough->undo();
}

void PassthroughPage::redo() {
    if (d->passthrough) d->passthrough->redo();
}

QCoro::Task<> PassthroughPage::save() {
    if (d->passthrough) {
        co_return co_await d->passthrough->save();
    }
}

QCoro::Task<> PassthroughPage::saveAs() {
    if (d->passthrough) {
        co_return co_await d->passthrough->saveAs();
    }
}

QCoro::Task<> PassthroughPage::saveAll() {
    if (d->passthrough) {
        co_return co_await d->passthrough->saveAll();
    }
}

QCoro::Task<> PassthroughPage::saveBeforeClose(bool silent) {
    if (d->passthrough) {
        co_return co_await d->passthrough->saveBeforeClose(silent);
    }
}

bool PassthroughPage::saveAndCloseShouldAskUserConfirmation() {
    if (d->passthrough) return d->passthrough->saveAndCloseShouldAskUserConfirmation();
    return false;
}
