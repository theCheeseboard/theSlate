#include "jsonsyntaxhighlighter.h"

JsonSyntaxHighlighter::JsonSyntaxHighlighter(QObject *parent) : SyntaxHighlighter(parent)
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

    //Key
    rule.pattern = QRegularExpression("\".+\"(?=:)");
    rule.format = classFormat;
    highlightingRules.append(rule);

}

void JsonSyntaxHighlighter::highlightBlock(const QString &text) {
    //Try to match strings
    int beginIndex = -1;
    for (int i = 0; i < text.count(); i++) {
        if (text.at(i) == '"') {
            if (beginIndex == -1) {
                beginIndex = i;
            } else {
                setFormat(beginIndex, i + 1 - beginIndex, quotationFormat);
                beginIndex = -1;
            }
        } else if (text.at(i) == '\\') {
            //Skip over the next character
            i++;
        }
    }
    if (beginIndex != -1) setFormat(beginIndex, text.count() - beginIndex, quotationFormat);

    for (HighlightingRule rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
