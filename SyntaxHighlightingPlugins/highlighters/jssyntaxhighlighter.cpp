#include "jssyntaxhighlighter.h"

JsSyntaxHighlighter::JsSyntaxHighlighter(QObject *parent) : SyntaxHighlighter(parent)
{
    QColor background = QApplication::palette("QPlainTextEditor").color(QPalette::Window);
    int avg = (background.blue() + background.green() + background.red()) / 3;
    if (avg > 127) {
        controlFormat.setForeground(Qt::blue);
        numberFormat.setForeground(QColor(200, 0, 0));
    } else {
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

    //Keywords
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\blet\\b" << "\\bfunction\\b";
    for (QString pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Controls
    QStringList controlPatterns;
    controlPatterns << "\\bif\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\bfor\\b" << "\\bswitch\\b" << "\\bcase\\b"
                    << "\\bof\\b" << "\\belse\\b" << "\\bconst\\b"
                    << "\\btry\\b" << "\\bcatch\\b" << "\\bin\\b"
                    << "\\bbreak\\b";
    for (QString pattern : controlPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = controlFormat;
        highlightingRules.append(rule);
    }

    //String
    //rule.pattern = QRegularExpression("\".*\"");
    //rule.format = quotationFormat;
    //highlightingRules.append(rule);

    //rule.pattern = QRegularExpression("'.*'");
    //highlightingRules.append(rule);

    //Function
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    //Comments
    rule.pattern = QRegularExpression("\\/\\*(\\w|\\s)*\\*\\/");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\/\\/(.)*");
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void JsSyntaxHighlighter::highlightBlock(const QString &text) {
    for (HighlightingRule rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    //Try to match strings
    int beginIndex = -1;
    QChar textDelimiter;
    for (int i = 0; i < text.count(); i++) {
        if (text.at(i) == '"' || text.at(i) == '\'' || text.at(i) == '`') {
            if (beginIndex == -1) {
                textDelimiter = text.at(i);
                beginIndex = i;
            } else if (text.at(i) == textDelimiter) {
                setFormat(beginIndex, i + 1 - beginIndex, quotationFormat);
                beginIndex = -1;
            }
        } else if (text.at(i) == '\\') {
            //Skip over the next character
            i++;
        }
    }
    if (beginIndex != -1) setFormat(beginIndex, text.count() - beginIndex, quotationFormat);
}
