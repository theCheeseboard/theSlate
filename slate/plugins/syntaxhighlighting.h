#ifndef SYNTAXHIGHLIGHTING_H
#define SYNTAXHIGHLIGHTING_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QApplication>
#include <QPalette>
#define THESLATE_SYNTAXHIGHLIGHTING_IID "org.thesuite.theSlate.SyntaxHighlighting"

class SyntaxHighlighter : public QSyntaxHighlighter {
    public:
        SyntaxHighlighter(QObject* parent) : QSyntaxHighlighter(parent) {

        }

        QColor getColor(QString color) {
            QColor background = QApplication::palette("QPlainTextEditor").color(QPalette::Window);
            int avg = (background.blue() + background.green() + background.red()) / 3;

            bool dark = false;
            if (avg < 127) {
                dark = true;
            }

            if (color == "comment") {
                return QColor(Qt::gray);
            } else if (color == "class") {
                if (dark) return QColor(Qt::magenta); else return QColor(Qt::darkMagenta);
            } else if (color == "string") {
                if (dark) return QColor(255, 100, 100); else return QColor(Qt::red);
            } else if (color == "function") {
                if (dark) return QColor(Qt::blue); else return QColor(0, 100, 255);
            } else if (color == "keyword") {
                return QColor(255, 150, 0);
            }
            return QColor();
        }

    private:
        virtual void highlightBlock(const QString &text) = 0;
};

class SyntaxHighlighting {
    public:
        virtual ~SyntaxHighlighting() {}

        virtual QStringList availableHighlighters() = 0;
        virtual SyntaxHighlighter* makeHighlighter(QString highlighter) = 0;
        virtual QString highlighterForFilename(QString filename) = 0;
};

Q_DECLARE_INTERFACE(SyntaxHighlighting, THESLATE_SYNTAXHIGHLIGHTING_IID)

#endif // SYNTAXHIGHLIGHTING_H
