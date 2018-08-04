#ifndef DEFAULTSYNTAXHIGHLIGHTING_H
#define DEFAULTSYNTAXHIGHLIGHTING_H

#include <QGenericPlugin>
#include "highlighters/cppsyntaxhighlighter.h"
#include "../slate/syntaxhighlighting/syntaxhighlighting.h"

class DefaultSyntaxHighlighting : public QObject,
                                  public SyntaxHighlighting
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID THESLATE_SYNTAXHIGHLIGHTING_IID FILE "SyntaxHighlightingPlugins.json")
        Q_INTERFACES(SyntaxHighlighting)

    public:
        DefaultSyntaxHighlighting();

        QStringList availableHighlighters() override;
        SyntaxHighlighter* makeHighlighter(QString highlighter) override;
};

#endif // DEFAULTSYNTAXHIGHLIGHTING_H
