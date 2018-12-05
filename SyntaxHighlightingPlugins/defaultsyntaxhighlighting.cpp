#include "defaultsyntaxhighlighting.h"

#include "highlighters/cppsyntaxhighlighter.h"
#include "highlighters/xmlsyntaxhighlighter.h"
#include "highlighters/jssyntaxhighlighter.h"
#include <QDebug>
#include <QApplication>
#include <QFileInfo>

DefaultSyntaxHighlighting::DefaultSyntaxHighlighting() : SyntaxHighlighting()
{

}

QStringList DefaultSyntaxHighlighting::availableHighlighters() {
    return QStringList() << "C++" << "XML" << "JavaScript";
}

SyntaxHighlighter* DefaultSyntaxHighlighting::makeHighlighter(QString highlighter) {
    if (highlighter == "C++") {
        return new CppSyntaxHighlighter(new QObject);
    } else if (highlighter == "XML") {
        return new XmlSyntaxHighlighter(new QObject);
    } else if (highlighter == "JavaScript") {
        return new JsSyntaxHighlighter(new QObject);
    }
    return nullptr;
}

QString DefaultSyntaxHighlighting::highlighterForFilename(QString filename) {
    QFileInfo info(filename);
    if (QStringList({"js"}).contains(info.suffix())) {
        return "JavaScript";
    } else if (QStringList({"c", "cpp", "h", "hpp"}).contains(info.suffix())) {
        return "C++";
    } else if (QStringList({"xml", "html"}).contains(info.suffix())) {
        return  "XML";
    }
    return "";
}
