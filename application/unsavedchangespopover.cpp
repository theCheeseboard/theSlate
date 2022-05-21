#include "unsavedchangespopover.h"
#include "ui_unsavedchangespopover.h"

#include "pages/abstractpage/abstractpage.h"
#include <tcontentsizer.h>

struct UnsavedChangesPopoverPrivate {
        QList<AbstractPage*> unsavedPages;
};

UnsavedChangesPopover::UnsavedChangesPopover(QList<AbstractPage*> unsavedPages, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UnsavedChangesPopover) {
    ui->setupUi(this);

    d = new UnsavedChangesPopoverPrivate();
    d->unsavedPages = unsavedPages;

    ui->titleLabel->setBackButtonShown(true);
    ui->discardAllChangesButton->setProperty("type", "destructive");

    new tContentSizer(ui->descriptionContainer);

    updateState();
}

UnsavedChangesPopover::~UnsavedChangesPopover() {
    delete ui;
    delete d;
}

void UnsavedChangesPopover::processNextFile() {
    if (d->unsavedPages.isEmpty()) {
        emit accepted();
        return;
    }

    AbstractPage* page = d->unsavedPages.at(0);
    page->save()->then([=] {
                    d->unsavedPages.removeOne(page);
                    processNextFile();
                })
        ->error([=](QString error) {
            updateState();
            emit show();
        });
}

void UnsavedChangesPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void UnsavedChangesPopover::on_saveAllChangesButton_clicked() {
    emit hide();
    processNextFile();
}

void UnsavedChangesPopover::on_discardAllChangesButton_clicked() {
    emit accepted();
}

void UnsavedChangesPopover::updateState() {
    ui->unsavedChangesLabel->setText(tr("Unsaved changes remain in %n files", nullptr, d->unsavedPages.length()));
}
