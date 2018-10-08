#include "topnotification.h"
#include "ui_topnotification.h"

TopNotification::TopNotification(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TopNotification)
{
    ui->setupUi(this);
}

TopNotification::~TopNotification()
{
    delete ui;
}

void TopNotification::setTitle(QString title) {
    ui->title->setText(title);
}

void TopNotification::setText(QString text) {
    ui->message->setText(text);
}

void TopNotification::addButton(QPushButton* button) {
    ui->buttonsLayout->addWidget(button);
    buttons.append(button);
}

QPushButton* TopNotification::firstButton() {
    if (buttons.count() > 1) {
        return buttons.first();
    }
    return nullptr;
}

QPushButton* TopNotification::secondButton() {
    if (buttons.count() == 1) {
        return buttons.first();
    } else if (buttons.count() > 1) {
        return buttons.at(1);
    }
    return nullptr;
}

void TopNotification::clearButtons() {
    while (buttons.count() > 0) {
        QPushButton* b = buttons.takeFirst();
        b->deleteLater();
        buttons.removeOne(b);
    }
}

void TopNotification::on_closeButton_clicked()
{
    emit closeNotification();
}
