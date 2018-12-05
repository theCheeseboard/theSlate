#ifndef DEFAULTSYNTAXHIGHLIGHTING_H
#define DEFAULTSYNTAXHIGHLIGHTING_H

#include <QGenericPlugin>
#include "../slate/plugins/syntaxhighlighting.h"

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
        QString highlighterForFilename(QString filename) override;
};

#endif // DEFAULTSYNTAXHIGHLIGHTING_H
