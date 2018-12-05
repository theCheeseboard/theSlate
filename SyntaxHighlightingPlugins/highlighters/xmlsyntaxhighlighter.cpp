#include "xmlsyntaxhighlighter.h"

XmlSyntaxHighlighter::XmlSyntaxHighlighter(QObject *parent) : SyntaxHighlighter(parent)
{   
    QColor background = QApplication::palette("QPlainTextEditor").color(QPalette::Window);
    int avg = (background.blue() + background.green() + background.red()) / 3;
    if (avg > 127) {
        preprocessorFormat.setForeground(QColor(150, 0, 0));
        controlFormat.setForeground(Qt::blue);
        numberFormat.setForeground(QColor(200, 0, 0));
    } else {
        preprocessorFormat.setForeground(QColor(150, 0, 0));
        controlFormat.setForeground(QColor(0, 100, 255));
        numberFormat.setForeground(QColor(255, 0, 0));
    }
    commentFormat.setForeground(getColor("comment"));
    classFormat.setForeground(getColor("class"));
    quotationFormat.setForeground(getColor("string"));
    functionFormat.setForeground(getColor("function"));
    keywordFormat.setForeground(getColor("keyword"));
    controlFormat.setFontWeight(90);

    HighlightingRule rule;

    //Tag (class)
    rule.pattern = QRegularExpression("(?<=<).+?(?=( |>))");
    rule.format = classFormat;
    highlightingRules.append(rule);

    //Attribute (function)
    rule.pattern = QRegularExpression("(?<= )[^<]*?(?==\")");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    //Attribute Value (quotation)
    rule.pattern = QRegularExpression("(?<==)\".*?\"(?=( |>))");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
}

void XmlSyntaxHighlighter::highlightBlock(const QString &text) {
    for (HighlightingRule rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
