#ifndef JSONSYNTAXHIGHLIGHTER_H
#define JSONSYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpressionMatchIterator>
#include <QPalette>
#include <QApplication>
#include "../slate/plugins/syntaxhighlighting.h"

class JsonSyntaxHighlighter : public SyntaxHighlighter
{
        Q_OBJECT
    public:
        explicit JsonSyntaxHighlighter(QColor (*getColor)(QString), QObject *parent = nullptr);

        enum State {
            Invalid = -1,
            Object
        };

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

#endif // XMLSYNTAXHIGHLIGHTER_H
