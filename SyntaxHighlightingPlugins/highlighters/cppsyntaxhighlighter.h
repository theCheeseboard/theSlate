#ifndef CPPSYNTAXHIGHLIGHTER_H
#define CPPSYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpressionMatchIterator>
#include <QPalette>
#include <QApplication>
#include "../slate/syntaxhighlighting/syntaxhighlighting.h"

class CppSyntaxHighlighter : public SyntaxHighlighter
{
        Q_OBJECT
    public:
        explicit CppSyntaxHighlighter(QObject *parent = nullptr);

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
};

#endif // CPPSYNTAXHIGHLIGHTER_H
