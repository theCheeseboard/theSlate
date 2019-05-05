#ifndef TEXTEDITORBLOCKDATA_H
#define TEXTEDITORBLOCKDATA_H

#include <QTextBlockUserData>
#include <FoldingRegion>
#include <State>

class TextEditor;
class SyntaxHighlighterBlockData;
class TextEditorBlockData : public QTextBlockUserData {
    public:
        enum MarginState {
            None,
            Edited,
            SavedEdited
        };

        TextEditorBlockData(TextEditor* parent);
        ~TextEditorBlockData();

        MarginState marginState = None;
        SyntaxHighlighterBlockData* syntaxHighlighterData;

        //For the syntax highlighter
        bool initializedSyntaxHighlighting = false;
        KSyntaxHighlighting::State state;
        QVector<KSyntaxHighlighting::FoldingRegion> foldingRegions;

    private:
        QMetaObject::Connection editedConnection;
};

#endif // TEXTEDITORBLOCKDATA_H
