#include "defaultsyntaxhighlighting.h"

#include <QDebug>

DefaultSyntaxHighlighting::DefaultSyntaxHighlighting() : SyntaxHighlighting()
{

}

QStringList DefaultSyntaxHighlighting::availableHighlighters() {
    return QStringList() << "C++" << "XML";
}

SyntaxHighlighter* DefaultSyntaxHighlighting::makeHighlighter(QString highlighter) {
    if (highlighter == "C++") {
        return new CppSyntaxHighlighter(new QObject);
    } else if (highlighter == "XML") {
        return new XmlSyntaxHighlighter(new QObject);
    }
    return nullptr;
}
