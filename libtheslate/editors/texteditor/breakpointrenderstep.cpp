#include "breakpointrenderstep.h"

#include "texteditor.h"
#include <QPainter>
#include <QRect>
#include <QScrollBar>
#include <libcontemporary_global.h>

BreakpointRenderStep::BreakpointRenderStep(TextEditor* parent) :
    TextEditorPerLineRenderStep{parent} {
}

TextEditorRenderStep::RenderSide BreakpointRenderStep::renderSide() const {
    return StackWithOther;
}

int BreakpointRenderStep::renderWidth() const {
    return SC_DPI_W(20, parentEditor());
}

QString BreakpointRenderStep::stepName() const {
    return "Breakpoint";
}

uint BreakpointRenderStep::priority() const {
    return LeftGutterText - 1;
}

QString BreakpointRenderStep::renderStack() const {
    return "LeftGutterText";
}

void BreakpointRenderStep::paintLine(int line, QPainter* painter, QRect outputBounds, QRect redrawBounds) {
    auto* editor = parentEditor();

    if (line % 3 == 0) {
        // Pretend there is a breakpoint here?
        painter->save();

        QRectF rect;
        rect.setHeight(editor->lineHeight(line));
        rect.setWidth(outputBounds.width());
        rect.moveTop(editor->lineTop(line) - editor->verticalScrollBar()->value());
        rect.moveLeft(outputBounds.left());
        // rect.adjust(SC_DPI_W(3, editor), SC_DPI_W(3, editor), SC_DPI_W(-3, editor), SC_DPI_W(-3, editor));

        QPolygonF poly;
        poly.append(rect.topLeft());
        poly.append(rect.topRight() - QPointF(rect.height() / 2, 0));
        poly.append(QPointF(rect.right(), rect.center().y()));
        poly.append(rect.bottomRight() - QPointF(rect.height() / 2, 0));
        poly.append(rect.bottomLeft());

        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(0, 118, 53));
        painter->drawPolygon(poly);
        painter->restore();
    }
}
