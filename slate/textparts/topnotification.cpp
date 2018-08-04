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
}
