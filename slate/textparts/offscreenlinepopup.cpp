#include "offscreenlinepopup.h"
#include "ui_offscreenlinepopup.h"

#include <QLabel>
#include "texteditor.h"

struct OffscreenLinePopupPrivate {
    TextEditor* editor;
    OffscreenLinePopup::Side side;

    QList<QLabel*> labels;
};

OffscreenLinePopup::OffscreenLinePopup(TextEditor* editor) :
    QWidget(editor),
    ui(new Ui::OffscreenLinePopup)
{
    ui->setupUi(this);

    d = new OffscreenLinePopupPrivate();
    d->editor = editor;

    this->setSide(Top);
    clearBlocks();
}

OffscreenLinePopup::~OffscreenLinePopup()
{
    delete ui;
    delete d;
}

void OffscreenLinePopup::setSide(Side side)
{
    if (side == Top) {
        this->move(SC_DPI(9), SC_DPI(9));
        this->layout()->setContentsMargins(1, SC_DPI(12) + 1, 1, 1);
    } else {
        this->move(SC_DPI(9), d->editor->height() - this->height() - SC_DPI(9));
        this->layout()->setContentsMargins(1, 1, 1, SC_DPI(12) + 1);
    }
    d->side = side;
}

void OffscreenLinePopup::setBlocks(QList<QTextBlock> blocks)
{
    for (QLabel* label : d->labels) {
        ui->blocksLayout->removeWidget(label);
        label->deleteLater();
    }
    d->labels.clear();

    for (QTextBlock block : blocks) {
        QTextLayout* layout = block.layout();

        QRectF geometry = d->editor->blockBoundingGeometry(block);
        geometry.setWidth(d->editor->width() - 2 - SC_DPI(18));

        QPixmap pixmap(geometry.size().toSize());
        pixmap.fill(d->editor->palette().color(QPalette::Window));
        QPainter painter(&pixmap);
        painter.setPen(d->editor->palette().color(QPalette::WindowText));

        QVector<QTextLayout::FormatRange> extraSelections;

        for (QTextEdit::ExtraSelection sel : d->editor->extraSelections()) {
            int start = sel.cursor.selectionStart() - block.position();
            int end = sel.cursor.selectionEnd() - block.position();

            if (start < block.length() && end > 0 && end > start) {
                QTextLayout::FormatRange rng;
                rng.start = start;
                rng.length = end - start;
                rng.format = sel.format;
                extraSelections.append(rng);
            }
        }

        layout->draw(&painter, QPointF(0, 0), extraSelections);

        painter.end();

        QLabel* label = new QLabel(this);
        label->setFixedSize(geometry.size().toSize());
        label->setVisible(true);
        label->setPixmap(pixmap);
        ui->blocksLayout->addWidget(label);
        d->labels.append(label);
    }

    this->resize(d->editor->width() - SC_DPI(18), this->sizeHint().height());
    this->setSide(d->side);

    this->raise();
    this->setVisible(true);
}

void OffscreenLinePopup::clearBlocks()
{
    this->setVisible(false);
}

void OffscreenLinePopup::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setBrush(d->editor->palette().color(QPalette::Window));
    painter.setPen(d->editor->palette().color(QPalette::WindowText));

    QPolygon pol;
    if (d->side == Top) {
        pol.append(QPoint(0, this->layout()->contentsMargins().top() - 1));
        pol.append(QPoint(SC_DPI(60), this->layout()->contentsMargins().top() - 1));
        pol.append(QPoint(SC_DPI(72), 0));
        pol.append(QPoint(SC_DPI(84), this->layout()->contentsMargins().top() - 1));
        pol.append(QPoint(this->width() - 1, this->layout()->contentsMargins().top() - 1));
        pol.append(QPoint(this->width() - 1, this->height() - this->layout()->contentsMargins().bottom()));
        pol.append(QPoint(0, this->height() - this->layout()->contentsMargins().bottom()));
        pol.append(QPoint(0, this->layout()->contentsMargins().top()));
    } else {
        pol.append(QPoint(0, 0));
        pol.append(QPoint(this->width() - 1, 0));
        pol.append(QPoint(this->width() - 1, this->height() - this->layout()->contentsMargins().bottom() + 1));
        pol.append(QPoint(SC_DPI(84), this->height() - this->layout()->contentsMargins().bottom() + 1));
        pol.append(QPoint(SC_DPI(72), this->height()));
        pol.append(QPoint(SC_DPI(60), this->height() - this->layout()->contentsMargins().bottom() + 1));
        pol.append(QPoint(0, this->height() - this->layout()->contentsMargins().bottom() + 1));
        pol.append(QPoint(0, 0));
    }
    painter.drawPolygon(pol);
}
