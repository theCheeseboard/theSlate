#include "statusbarbutton.h"

#include <QPainter>
#include <QStyleOptionToolButton>
#include <the-libs_global.h>

StatusBarButton::StatusBarButton(QWidget *parent) : QToolButton(parent)
{
    this->setMouseTracking(true);
    this->setAutoRaise(true);
    this->setCursor(QCursor(Qt::PointingHandCursor));
    this->setPopupMode(InstantPopup);
}

void StatusBarButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    QStyleOptionToolButton style;
    this->initStyleOption(&style);

    if (style.state & QStyle::State_Sunken) {
        //Draw press
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.drawRect(0, 0, this->width(), this->height());
    } else if (style.state & QStyle::State_MouseOver) {
        //Draw hover
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(255, 255, 255, 100));
        painter.drawRect(0, 0, this->width(), this->height());
    }

    QRect textRect;
    textRect.setWidth(this->fontMetrics().width(this->text()) + 1);
    textRect.setHeight(this->fontMetrics().height());
    textRect.moveTo(3, 3);

    painter.setFont(this->font());
    painter.setBrush(Qt::transparent);
    painter.setPen(this->palette().color(QPalette::Text));
    painter.drawText(textRect, this->text());
}

QSize StatusBarButton::sizeHint() const {
    return QSize(this->fontMetrics().width(this->text()) + 1 + 6 * theLibsGlobal::getDPIScaling(), this->fontMetrics().height() + 6 * theLibsGlobal::getDPIScaling());
}
