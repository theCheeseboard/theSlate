#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QFileSystemWatcher>
#include <QSyntaxHighlighter>
#include "tabbutton.h"
#include "SourceControl/gitintegration.h"
#include "textparts/findreplace.h"
#include "textparts/topnotification.h"
#include "textparts/mergetool.h"

class TabButton;
class FindReplace;
class MainWindow;
struct MergeLines;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

    class TextEditorLeftMargin : public QWidget
    {
        public:
            TextEditorLeftMargin(TextEditor *editor) : QWidget(editor) {
                this->editor = editor;
                this->setCursor(Qt::ArrowCursor);
            }

            QSize sizeHint() const override {
                return QSize(editor->leftMarginWidth(), 0);
            }

        protected:
            void paintEvent(QPaintEvent *event) override {
                editor->leftMarginPaintEvent(event);
            }

        private:
            void mousePressEvent(QMouseEvent* event) override {
                int lineNo = editor->cursorForPosition(event->pos()).block().firstLineNumber() + 1;
                editor->toggleMergedLines(lineNo);
            }

            TextEditor *editor;
    };

    public:
        explicit TextEditor(MainWindow *parent);
        ~TextEditor();

        QString filename();
        bool isEdited();
        QSyntaxHighlighter* highlighter();

        void leftMarginPaintEvent(QPaintEvent *event);
        int leftMarginWidth();

        GitIntegration* git = nullptr;

    signals:
        void fileNameChanged();
        void editedChanged();
        void mergeDecision(MergeLines lines, bool on);

    public slots:
        TabButton* getTabButton();
        void setActive(bool active);

        void openFile(QString file);
        void openFileFake(QString filename, QString contents);
        bool saveFile(QString file);
        bool saveFile();
        bool saveFileAskForFilename(bool saveAs = false);
        void revertFile();

        void setHighlighter(QSyntaxHighlighter* hl);

        void setExtraSelectionGroup(QString extraSelectionGroup, QList<QTextEdit::ExtraSelection> selections);
        QList<QTextEdit::ExtraSelection> extraSelectionGroup(QString extraSelectionGroup);
        void clearExtraSelectionGroup(QString extraSelectionGroups);

        void toggleFindReplace();
        void lockScrolling(TextEditor* other);
        void setMergedLines(QList<MergeLines> mergedLines);
        bool mergedLineIsAccepted(MergeLines mergedLine);
        void toggleMergedLines(int line);
        void updateMergedLinesColour();

    private slots:
        void updateLeftMarginAreaWidth();
        void highlightCurrentLine();
        void updateLeftMarginArea(const QRect &, int);
        void reloadBlockHighlighting();

        void updateExtraSelections();
        void setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections);

        void addTopPanel(QWidget* panel);
        void removeTopPanel(QWidget* panel);

        void fileOnDiskChanged();

    private:
        TabButton* button;
        bool active;
        bool edited = false;
        bool firstEdit = true;
        QString fn;
        QSyntaxHighlighter* hl = nullptr;
        MainWindow* parentWindow;

        void keyPressEvent(QKeyEvent* event);
        void resizeEvent(QResizeEvent* event);

        void dragEnterEvent(QDragEnterEvent* event);
        void dragLeaveEvent(QDragLeaveEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);
        QTextCursor cursorBeforeDrop;

        TextEditorLeftMargin *leftMargin = NULL;
        int brokenLine = -1;

        FindReplace* findReplaceWidget;
        QMap<QString, QList<QTextEdit::ExtraSelection>> extraSelectionGroups;

        QList<QWidget*> topPanels;
        QWidget* topPanelWidget;
        QBoxLayout* topPanelLayout;

        TopNotification *mergeConflictsNotification, *onDiskChanged;
        QFileSystemWatcher* fileWatcher;

        TextEditor* scrollingLock = nullptr;

        int highlightedLine = -1;
        QList<MergeLines> mergedLines;
        QMap<MergeLines, bool> mergeDecisions;
};

#endif // TEXTEDITOR_H
