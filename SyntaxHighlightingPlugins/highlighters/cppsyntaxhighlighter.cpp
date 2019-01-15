#include "cppsyntaxhighlighter.h"

CppSyntaxHighlighter::CppSyntaxHighlighter(QColor (*getColor)(QString), QObject *parent) : SyntaxHighlighter(getColor, parent)
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

    //Keywords
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\busing\\b";
    for (QString pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Controls
    QStringList controlPatterns;
    controlPatterns << "\\bif\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\bfor\\b" << "\\bswitch\\b" << "\\bcase\\b"
                    << "\\belse\\b" << "\\breturn\\b";
    for (QString pattern : controlPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = controlFormat;
        highlightingRules.append(rule);
    }

    //Class
    rule.pattern = QRegularExpression("\\b[A-Za-z]+(?=(\\.|->))\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("(?<=class )([A-Z]|[a-z])([A-Z]|[a-z]|\\d)*");
    highlightingRules.append(rule);

    //String
    //rule.pattern = QRegularExpression("\".*\"");
    //rule.format = quotationFormat;
    //highlightingRules.append(rule);

    //Function
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    //Preprocessor
    rule.pattern = QRegularExpression("#.+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    //Comments
    rule.pattern = QRegularExpression("\\/\\*(\\w|\\s)*\\*\\/");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("\\/\\/(.)*");
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*//*");
}

void CppSyntaxHighlighter::highlightBlock(const QString &text) {
    for (HighlightingRule rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

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
}
