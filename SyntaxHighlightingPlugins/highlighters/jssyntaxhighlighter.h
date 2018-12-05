#ifndef JSSYNTAXHIGHLIGHTER_H
#define JSSYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpressionMatchIterator>
#include <QPalette>
#include <QApplication>
#include "../slate/plugins/syntaxhighlighting.h"

class JsSyntaxHighlighter : public SyntaxHighlighter
{
        Q_OBJECT
    public:
        explicit JsSyntaxHighlighter(QObject *parent = nullptr);

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
        QTextCharFormat numberFormat;
};

#endif // CPPSYNTAXHIGHLIGHTER_H
