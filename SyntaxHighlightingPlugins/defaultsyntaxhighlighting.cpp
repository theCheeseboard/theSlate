#include "defaultsyntaxhighlighting.h"

#include <QDebug>

DefaultSyntaxHighlighting::DefaultSyntaxHighlighting() : SyntaxHighlighting()
{

}

QStringList DefaultSyntaxHighlighting::availableHighlighters() {
    return QStringList() << "C++";
}

SyntaxHighlighter* DefaultSyntaxHighlighting::makeHighlighter(QString highlighter) {
    if (highlighter == "C++") {
        return new CppSyntaxHighlighter(new QObject);
    }
    return nullptr;
}
