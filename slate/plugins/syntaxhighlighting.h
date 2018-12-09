#ifndef SYNTAXHIGHLIGHTING_H
#define SYNTAXHIGHLIGHTING_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QApplication>
#include <QPalette>
#define THESLATE_SYNTAXHIGHLIGHTING_IID "org.thesuite.theSlate.SyntaxHighlighting"

extern QColor getColor(QString color);

class SyntaxHighlighter : public QSyntaxHighlighter {
    public:
        SyntaxHighlighter(QColor (*getColor)(QString), QObject* parent) : QSyntaxHighlighter(parent) {

        }


    private:
        virtual void highlightBlock(const QString &text) = 0;
        QColor (*getColor)(QString);

};

class SyntaxHighlighting {
    public:
        virtual ~SyntaxHighlighting() {}

        virtual QStringList availableHighlighters() = 0;
        virtual SyntaxHighlighter* makeHighlighter(QString highlighter, QColor (*getColor)(QString)) = 0;
        virtual QString highlighterForFilename(QString filename) = 0;
};

Q_DECLARE_INTERFACE(SyntaxHighlighting, THESLATE_SYNTAXHIGHLIGHTING_IID)

#endif // SYNTAXHIGHLIGHTING_H
