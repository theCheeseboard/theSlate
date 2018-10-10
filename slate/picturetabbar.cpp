#include "picturetabbar.h"
#include "ui_picturetabbar.h"

PictureTabBar::PictureTabBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PictureTabBar)
{
    ui->setupUi(this);
}

PictureTabBar::~PictureTabBar()
{
    delete ui;
}

QSize PictureTabBar::sizeHint() const {
    return QSize(100, 100);
}

void PictureTabBar::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setBrush(Qt::red);
    painter.drawRect(0, 0, this->width(), this->height());
}
