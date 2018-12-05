#ifndef XMLSYNTAXHIGHLIGHTER_H
#define XMLSYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpressionMatchIterator>
#include <QPalette>
#include <QApplication>
#include "../slate/plugins/syntaxhighlighting.h"
#include "jssyntaxhighlighter.h"

class XmlSyntaxHighlighter : public SyntaxHighlighter
{
        Q_OBJECT
    public:
        explicit XmlSyntaxHighlighter(QObject *parent = nullptr);

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

        JsSyntaxHighlighter* jsHighlighter;
};

#endif // XMLSYNTAXHIGHLIGHTER_H
