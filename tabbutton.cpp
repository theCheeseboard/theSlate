#include "tabbutton.h"

TabButton::TabButton(QWidget *parent) : QPushButton(parent)
{
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
}

TabButton::TabButton(TextEditor *editor, QWidget *parent) : QPushButton(parent)
{
    this->editor = editor;
}

void TabButton::setActive(bool active) {
    this->active = active;
    this->repaint();
}

void TabButton::updateIcon(QIcon icon) {
    this->setIcon(icon);
}

void TabButton::updateTitle(QString title) {
    this->setText(title);
}

void TabButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPalette pal = this->palette();
    QRect rect = event->rect();

    painter.setPen(Qt::transparent);

    QBrush brush;
    QPen textPen;

#ifdef Q_OS_WIN
    brush = QBrush(pal.color(QPalette::Highlight));
#elif Q_OS_LINUX
    brush = QBrush(pal.color(QPalette::Button));
#endif

    /*if (button->state & QStyle::State_HasFocus) {
        brush = QBrush(pal.color(QPalette::Button).lighter(125));
    }

    if (button->state & QStyle::State_MouseOver) {
        brush = QBrush(pal.color(QPalette::Button).lighter());
    }*/

    if (this->active) {
#ifdef Q_OS_WIN
        brush = QBrush(pal.color(QPalette::Highlight).darker());
#elif Q_OS_LINUX
        brush = QBrush(pal.color(QPalette::Button).darker());
#endif
    }
#ifdef Q_OS_WIN
    textPen = pal.color(QPalette::HighlightedText);
#elif Q_OS_LINUX
    textPen = pal.color(QPalette::ButtonText);
#endif

    painter.setBrush(brush);
    painter.drawRect(rect);

    QString text = this->text();

    QRect textRect, iconRect;
    textRect.setLeft(rect.left() + (rect.width() / 2) - (this->fontMetrics().width(text) / 2));
    textRect.setWidth(this->fontMetrics().width(text));
    textRect.setTop(rect.top() + (rect.height() / 2) - (this->fontMetrics().height() / 2));
    textRect.setHeight(this->fontMetrics().height());

    if (!this->icon().isNull()) {
        int fullWidth = textRect.width() + this->iconSize().width();
        int iconLeft = rect.left() + (rect.width() / 2) - (fullWidth / 2);

        iconRect.setLeft(iconLeft);
        iconRect.setTop(rect.top() + (rect.height() / 2) - (this->iconSize().height() / 2));
        iconRect.setSize(this->iconSize());

        textRect.moveLeft(iconRect.right() + 4);

        QIcon icon = this->icon();
        QImage image = icon.pixmap(this->iconSize()).toImage();
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        painter.drawImage(iconRect, image);
    }

    //Draw text
    painter.setPen(textPen);
    painter.drawText(textRect, Qt::AlignCenter, text.remove("&"));
}

QSize TabButton::sizeHint() const {
    QSize size;
    size.setHeight(this->fontMetrics().height() + 20);
    //size.setWidth(this->fontMetrics().width(this->text()) + 20 + this->iconSize().width());
    size.setWidth(size.height());
    return size;
}
