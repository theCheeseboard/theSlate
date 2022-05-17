#include "texteditor.h"

#include "../abstracteditor/abstracteditorcolorscheme.h"
#include "textcaret.h"
#include <QFontDatabase>
#include <QPainter>
#include <QRandomGenerator64>
#include <QScrollBar>
#include <QWheelEvent>
#include <libcontemporary_global.h>

#include "texteditor_p.h"

TextEditor::TextEditor(QWidget* parent) :
    AbstractEditor{parent} {
    d = new TextEditorPrivate();

    this->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::StrongFocus);

    d->lines.append(new TextEditorPrivate::Line{"Line 1 Text"});
    d->lines.append(new TextEditorPrivate::Line{"Line 2 Text"});
    d->lines.append(new TextEditorPrivate::Line{"Line 3 Text"});
    d->lines.append(new TextEditorPrivate::Line{"Line 4 Text"});
    d->lines.append(new TextEditorPrivate::Line{"Line 5 Text"});

    d->vScrollBar = new QScrollBar(Qt::Vertical);
    d->vScrollBar->setFixedWidth(this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    d->vScrollBar->setParent(this);
    connect(d->vScrollBar, &QScrollBar::valueChanged, this, QOverload<>::of(&TextEditor::repaint));
    d->vScrollBar->show();

    d->hScrollBar = new QScrollBar(Qt::Horizontal);
    d->hScrollBar->setFixedHeight(this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    d->hScrollBar->setParent(this);
    connect(d->hScrollBar, &QScrollBar::valueChanged, this, QOverload<>::of(&TextEditor::repaint));
    d->hScrollBar->show();

    addCaret(0, 0);
    addCaret(1, 1);
    addCaret(2, 5);
    d->carets.first()->setIsPrimary(true);

    this->repositionElements();
}

TextEditor::~TextEditor() {
    delete d;
}

void TextEditor::repositionElements() {
    QRect vsGeometry;
    vsGeometry.setWidth(d->vScrollBar->width());
    vsGeometry.setHeight(this->height() - this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    vsGeometry.moveTop(0);
    vsGeometry.moveRight(this->width());
    d->vScrollBar->setGeometry(vsGeometry);
    d->vScrollBar->setPageStep(this->height());

    QRect hsGeometry;
    hsGeometry.setHeight(d->hScrollBar->height());
    hsGeometry.setWidth(this->width() - this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    hsGeometry.moveLeft(0);
    hsGeometry.moveBottom(this->height());
    d->hScrollBar->setGeometry(hsGeometry);
    d->hScrollBar->setPageStep(this->width());
}

int TextEditor::leftMarginWidth() {
    QString testString;
    testString.fill('0', QString::number(d->lines.length()).length());
    return SC_DPI_W(20, this) + this->fontMetrics().horizontalAdvance(testString);
}

int TextEditor::lineTop(int line) {
    if (d->lineTops.length() > line) return d->lineTops.value(line);

    int top;
    if (line == 0) {
        top = 0;
    } else {
        top = lineTop(line - 1) + lineHeight(line);
    }
    d->lineTops.append(top);
    return top;
}

int TextEditor::lineHeight(int line) {
    return this->fontMetrics().height() + SC_DPI_W(4, this);
}

int TextEditor::firstLineOnScreen() {
    for (int i = 0; i < d->lines.length(); i++) {
        if (lineHeight(i) + lineTop(i) > d->vScrollBar->value()) return i;
    }
    return 0;
}

int TextEditor::lastLineOnScreen() {
    for (int i = firstLineOnScreen(); i < d->lines.length(); i++) {
        if (lineTop(i) > d->vScrollBar->value() + this->height()) return i - 1;
    }
    return d->lines.length() - 1;
}

void TextEditor::drawLine(int line, QPainter* painter) {
    QRect lineRect;
    lineRect.setHeight(lineHeight(line));
    lineRect.setWidth(this->width());
    lineRect.moveTop(lineTop(line) - d->vScrollBar->value());
    lineRect.moveLeft(0 - d->hScrollBar->value());

    QRect margin = lineRect;
    margin.setWidth(this->leftMarginWidth());

    QFont marginFont = this->font();
    marginFont.setPointSizeF(this->font().pointSizeF() * 0.8);

    bool isActiveLine = false;
    for (TextCaret* caret : d->carets) {
        if (caret->linePos().y() == line) {
            isActiveLine = true;
            break;
        }
    }

    painter->save();

    // Draw margin elements
    //    if (line == 16) {
    //        // Breakpoint
    //        QPolygon poly;
    //        poly.append(margin.topLeft());
    //        poly.append(margin.topRight());
    //        poly.append(QPoint(margin.right() + margin.height() / 2, margin.center().y()));
    //        poly.append(margin.bottomRight());
    //        poly.append(margin.bottomLeft());

    //        painter->setPen(Qt::transparent);
    //        painter->setBrush(this->colorScheme()->item(AbstractEditorColorScheme::Breakpoint).color());
    //        painter->drawPolygon(poly);
    //    }

    // Draw active line highlight
    if (isActiveLine) {
        painter->fillRect(lineRect, this->colorScheme()->item(AbstractEditorColorScheme::ActiveLine).color());
        painter->setPen(this->colorScheme()->item(AbstractEditorColorScheme::ActiveLineMarginText).color());
    } else {
        painter->setPen(this->colorScheme()->item(AbstractEditorColorScheme::MarginText).color());
    }

    painter->setFont(marginFont);

    QRect marginTextRect;

    QString marginText = QString::number(line + 1);
    marginTextRect.setWidth(QFontMetrics(marginFont).horizontalAdvance(marginText));
    marginTextRect.setHeight(QFontMetrics(marginFont).height());
    marginTextRect.moveCenter(margin.center());
    marginTextRect.moveRight(margin.right() - SC_DPI_W(3, this));
    painter->drawText(marginTextRect, Qt::AlignCenter, marginText);

    painter->setFont(this->font());
    QRect lineTextRect;
    QString lineText = d->lines.at(line)->contents;
    lineTextRect.setWidth(this->fontMetrics().horizontalAdvance(lineText));
    lineTextRect.setHeight(this->fontMetrics().height());
    lineTextRect.moveCenter(lineRect.center());
    lineTextRect.moveLeft(margin.right() + SC_DPI_W(3, this));
    painter->drawText(lineTextRect, Qt::AlignVCenter | Qt::AlignLeft, lineText);

    painter->restore();
}

void TextEditor::addCaret(int line, int pos) {
    TextCaret* caret = new TextCaret(line, pos, this);
    connect(caret, &TextCaret::destroyed, this, [=] {
        d->carets.removeAll(caret);
    });
    d->carets.append(caret);
}

void TextEditor::addCaret(QPoint linePos) {
    addCaret(linePos.y(), linePos.x());
}

void TextEditor::simplifyCarets() {
    QList<QPoint> foundCarets;
    for (TextCaret* caret : d->carets) {
        QPoint linePos = caret->linePos();
        if (foundCarets.contains(linePos)) {
            caret->deleteLater();
        } else {
            foundCarets.append(linePos);
        }
    }
}

QPoint TextEditor::hitTest(QPoint pos) {
    int translatedMouseX = pos.x() - leftMarginWidth() + d->hScrollBar->value();
    int translatedMouseY = pos.y() + d->vScrollBar->value();

    int selectedLine = this->lastLineOnScreen();
    for (int i = firstLineOnScreen(); i <= this->lastLineOnScreen(); i++) {
        int top = lineTop(i);
        int bottom = top + lineHeight(i);
        if (translatedMouseY >= top && translatedMouseY < bottom) {
            selectedLine = i;
            break;
        }
    }

    QString lineContents = d->lines.at(selectedLine)->contents;
    int lastX = -30000;
    for (int i = 0; i < lineContents.length(); i++) {
        int x = this->fontMetrics().horizontalAdvance(lineContents.left(i));
        if (x > translatedMouseX) {
            if (translatedMouseX - lastX > x - translatedMouseX) {
                return QPoint(i, selectedLine);
            } else {
                return QPoint(i - 1, selectedLine);
            }
        }
        lastX = x;
    }

    return QPoint(lineContents.length(), selectedLine);
}

int TextEditor::linePosToChar(QPoint linePos) {
    int c = linePos.x();
    for (int i = 0; i < linePos.y(); i++) {
        c += d->lines.at(i)->contents.length() + 1;
    }
    return c;
}

QPoint TextEditor::charToLinePos(int c) {
    if (c < 0) c = 0;

    int charsLeft = c;
    for (int i = 0; i < d->lines.length(); i++) {
        c -= d->lines.at(i)->contents.length() + 1;
        if (c < 0) {
            return QPoint(d->lines.at(i)->contents.length() + c + 1, i);
        }
    }
    return QPoint(d->lines.last()->contents.length() + 1, d->lines.length() - 1);
}

void TextEditor::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(QRect(0, 0, this->width(), this->height()), this->colorScheme()->item(AbstractEditorColorScheme::Background));

    QRect margin;
    margin.setTop(0);
    margin.setLeft(-d->hScrollBar->value());
    margin.setHeight(this->height());
    margin.setWidth(this->leftMarginWidth());
    painter.fillRect(margin, this->colorScheme()->item(AbstractEditorColorScheme::Margin));

    for (int i = this->firstLineOnScreen(); i <= this->lastLineOnScreen(); i++) {
        drawLine(i, &painter);
    }

    for (TextCaret* caret : d->carets) {
        caret->drawCaret(&painter);
    }
}

void TextEditor::resizeEvent(QResizeEvent* event) {
    this->repositionElements();
}

void TextEditor::wheelEvent(QWheelEvent* event) {
    d->vScrollBar->setValue(d->vScrollBar->value() - event->pixelDelta().y());
    d->hScrollBar->setValue(d->hScrollBar->value() - event->pixelDelta().x());
}

void TextEditor::mousePressEvent(QMouseEvent* event) {
    event->accept();

    // Set the caret
    if (event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
        addCaret(hitTest(event->pos()));
    } else {
        for (TextCaret* caret : d->carets) {
            if (caret->isPrimary()) {
                caret->moveCaret(hitTest(event->pos()));
            } else {
                caret->deleteLater();
            }
        }
    }
}

void TextEditor::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();
}

void TextEditor::mouseMoveEvent(QMouseEvent* event) {
    if (event->pos().x() + d->hScrollBar->value() < leftMarginWidth()) {
        this->setCursor(QCursor(Qt::ArrowCursor));
    } else {
        this->setCursor(QCursor(Qt::IBeamCursor));
    }
}