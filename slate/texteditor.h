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
#include <QTextCodec>
#include "tabbutton.h"
#include "SourceControl/gitintegration.h"
#include "textparts/findreplace.h"
#include "textparts/topnotification.h"
#include "textparts/mergetool.h"

class TabButton;
class FindReplace;
class MainWindow;
class FileBackend;
struct MergeLines;

class TextEditorPrivate;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

    public:
        explicit TextEditor(MainWindow *parent);
        ~TextEditor();

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

        QUrl fileUrl();
        QString title();
        bool isEdited();
        QSyntaxHighlighter* highlighter();

        void leftMarginPaintEvent(QPaintEvent *event);
        int leftMarginWidth();

        GitIntegration* git = nullptr;

    signals:
        void backendChanged();
        void editedChanged();
        void mergeDecision(MergeLines lines, bool on);
        void titleChanged(QString title);
        void primaryTopNotificationChanged(TopNotification* topNotification);

    public slots:
        TabButton* getTabButton();
        void setActive(bool active);

        void openFile(FileBackend* backend);
        void openFileFake(QString contents);
        void loadText(QByteArray data);
        bool saveFile();
        bool saveFileAskForFilename(bool saveAs = false);
        void revertFile(QTextCodec* codec = nullptr);

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

        void setTextCodec(QTextCodec* codec);

        void reloadSettings();

    private slots:
        void updateLeftMarginAreaWidth();
        void highlightCurrentLine();
        void cursorLocationChanged();
        void updateLeftMarginArea(const QRect &, int);
        void reloadBlockHighlighting();

        void updateExtraSelections();
        void setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections);

        void addTopPanel(QWidget* panel);
        void removeTopPanel(QWidget* panel);

        void connectBackend();
        QByteArray formatForSaving(QString text);

    private:
        TextEditorPrivate* d;

        void keyPressEvent(QKeyEvent* event);
        void resizeEvent(QResizeEvent* event);

        void dragEnterEvent(QDragEnterEvent* event);
        void dragLeaveEvent(QDragLeaveEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);
};

#endif // TEXTEDITOR_H
