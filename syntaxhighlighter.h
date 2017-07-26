#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QApplication>
#include <QPalette>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    enum codeType {
        none,
        cpp,
        js,
        xml,
        md,
        py,
        json
    };

    explicit SyntaxHighlighter(QTextDocument *parent = 0);

    void setCodeType(codeType type);
    codeType currentCodeType();
signals:

public slots:

private:
    void highlightBlock(const QString &text) override;

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat controlFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat numberFormat;

    codeType ct = none;
};

#endif // SYNTAXHIGHLIGHTER_H
