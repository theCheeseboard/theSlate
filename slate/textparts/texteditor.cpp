#include "texteditor.h"

#include <QStyle>
#include <tmessagebox.h>
#include <QMimeData>
#include <QScrollBar>
#include <QMenuBar>
#include <QSignalBlocker>
#include <QMimeDatabase>
#include <QInputDialog>
#include <tcircularspinner.h>
#include <ttoast.h>
#include "the-libs_global.h"
#include "mainwindow.h"
#include "plugins/pluginmanager.h"
#include "managers/recentfilesmanager.h"
#include "textparts/textstatusbar.h"
#include "textparts/selectlistdialog.h"
#include "texteditorblockdata.h"

#include <Repository>
#include "SyntaxHighlighting/syntaxhighlighter.h"
#include <Theme>

#if THE_LIBS_API_VERSION >= 3
#include <tpopover.h>
#endif

extern PluginManager* plugins;
extern RecentFilesManager* recentFiles;
extern KSyntaxHighlighting::Repository* highlightRepo;
extern KSyntaxHighlighting::Theme highlightTheme;

class TextEditorPrivate {
    public:
        TabButton* button;
        bool active;
        bool edited = false;
        bool firstEdit = true;
        SyntaxHighlighter* hl = nullptr;
        KSyntaxHighlighting::Definition hlDef;
        MainWindow* parentWindow;

        QTextCodec* textCodec = nullptr;

        QTextCursor cursorBeforeDrop;

        TextEditorLeftMargin *leftMargin = nullptr;
        int brokenLine = -1;

        QMap<QString, QList<QTextEdit::ExtraSelection>> extraSelectionGroups;

        QList<QWidget*> topPanels;
        QWidget* topPanelWidget;
        QBoxLayout* topPanelLayout;

        TopNotification *mergeConflictsNotification, *onDiskChanged, *fileReadError, *onDiskDeleted, *mixedLineEndings, *decodingProblem;

        TextEditor* scrollingLock = nullptr;

        int highlightedLine = -1;
        QList<MergeLines> mergedLines;

        QSettings settings;
        QMap<MergeLines, bool> mergeDecisions;

        QWidget* cover;

        FileBackend* currentBackend = nullptr;
        TextStatusBar* statusBar = nullptr;

        QString getIndentCharacters() {
            QString spacingCharacters;
            bool tabSpaces = settings.value("behaviour/tabSpaces", true).toBool();
            if (tabSpaces) {
                spacingCharacters = QString().fill(' ', settings.value("behaviour/tabSpaceNumber", 4).toInt());
            } else {
                spacingCharacters = "\t";
            }
            return spacingCharacters;
        }

        void forEverySelectedLine(QTextCursor selection, std::function<void (QTextCursor)> function) {
            QTextCursor startCursor(selection.document());
            startCursor.setPosition(selection.selectionStart());

            if (selection.hasSelection()) {
                QTextCursor endCursor(selection.document());
                endCursor.setPosition(selection.selectionEnd());

                startCursor.beginEditBlock();

                //Comment each line
                bool couldMove;
                do {
                    startCursor.movePosition(QTextCursor::StartOfBlock);
                    function(startCursor);
                    couldMove = startCursor.movePosition(QTextCursor::NextBlock);
                } while (startCursor.blockNumber() != endCursor.blockNumber() || !couldMove);

                if (couldMove) {
                    //Comment one more line
                    startCursor.movePosition(QTextCursor::StartOfBlock);
                    function(startCursor);
                }

                startCursor.endEditBlock();
            } else {
                startCursor.beginEditBlock();
                startCursor.movePosition(QTextCursor::StartOfBlock);
                function(startCursor);
                startCursor.endEditBlock();
            }
        }
};

TextEditor::TextEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    d = new TextEditorPrivate();
    this->setLineWrapMode(NoWrap);

    QFont normalFont = this->font();

    d->button = new TabButton(this);
    connect(d->button, &TabButton::destroyed, [=] {
        d->button = nullptr;
    });
    d->button->setText(tr("New Document"));

    connect(this, &TextEditor::textChanged, [=] {
        if (d->firstEdit) {
            d->firstEdit = false;
        } else {
            if (this->textCursor().block().userData() == nullptr) this->textCursor().block().setUserData(new TextEditorBlockData(this));
            ((TextEditorBlockData*) this->textCursor().block().userData())->marginState = TextEditorBlockData::Edited;
            d->leftMargin->update();
            d->edited = true;
            emit editedChanged();
        }
    });
    connect(this, &TextEditor::backendChanged, [=] {
        d->button->setText(this->title());
        emit titleChanged(this->title());
    });

    d->leftMargin = new TextEditorLeftMargin(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEditor::updateLeftMarginAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditor::updateLeftMarginArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEditor::cursorLocationChanged);

    d->leftMargin->setVisible(true);

    d->cover = new QWidget();
    d->cover->setParent(this);
    d->cover->move(0, 0);
    d->cover->setVisible(false);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Expanding));
    layout->addWidget(new tCircularSpinner());
    layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Expanding));
    d->cover->setLayout(layout);

    d->topPanelWidget = new QWidget(this);
    d->topPanelWidget->move(0, 0);
    d->topPanelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    d->topPanelWidget->setFixedWidth(this->width());
    d->topPanelLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    d->topPanelLayout->setContentsMargins(0, 1, 0, 0);
    d->topPanelWidget->setLayout(d->topPanelLayout);

    {
        d->mergeConflictsNotification = new TopNotification();
        d->mergeConflictsNotification->setFont(normalFont);
        d->mergeConflictsNotification->setTitle("Merge Conflicts");
        d->mergeConflictsNotification->setText(tr("Merge Conflicts were found in this file"));

        QPushButton* fixMergeButton = new QPushButton();
        fixMergeButton->setText(tr("Resolve Merge Conflicts"));
        fixMergeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(fixMergeButton, &QPushButton::clicked, [=] {
            //Open the merge resolution tool
            MergeTool* tool = new MergeTool(this->toPlainText(), d->hlDef, d->parentWindow, this->window());
            tool->setTitle(tr("Resolve a Merge Conflict"));
            tool->resize(this->width() - 20, this->height() - 50);
            //#if THE_LIBS_API_VERSION >= 3 && !defined(Q_OS_MAC)
                tPopover* popover = new tPopover(tool);
                popover->setPopoverWidth(-100 * theLibsGlobal::getDPIScaling());
                popover->show(this->window());

                connect(tool, &MergeTool::acceptResolution, [=](QString revisedFile) {
                    this->setPlainText(revisedFile);
                    removeTopPanel(d->mergeConflictsNotification);
                });
                connect(tool, &MergeTool::finished, [=] {
                    d->parentWindow->menuBar()->setEnabled(true);
                    popover->dismiss();
                });
                connect(popover, &tPopover::dismissed, [=] {
                    tool->reject();
                });
            /*#else
                tool->setWindowFlag(Qt::Sheet);
                tool->setWindowModality(Qt::WindowModal);
                tool->show();
                d->parentWindow->menuBar()->setEnabled(false);

                connect(tool, &MergeTool::acceptResolution, [=](QString revisedFile) {
                    this->setPlainText(revisedFile);
                    removeTopPanel(d->mergeConflictsNotification);
                });
                connect(tool, &MergeTool::finished, [=] {
                    d->parentWindow->menuBar()->setEnabled(true);
                });
            #endif*/
        });;
        d->mergeConflictsNotification->addButton(fixMergeButton);

        connect(d->mergeConflictsNotification, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->mergeConflictsNotification);
        });
    }

    {
        d->onDiskChanged = new TopNotification();
        d->onDiskChanged->setFont(normalFont);
        d->onDiskChanged->setTitle(tr("File on disk changed"));
        d->onDiskChanged->setText(tr("The file on the disk has changed. If you save this file you will lose the changes on disk."));

        QPushButton* reloadButton = new QPushButton();
        reloadButton->setText(tr("Reload File"));
        reloadButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(reloadButton, SIGNAL(clicked(bool)), this, SLOT(revertFile()));
        d->onDiskChanged->addButton(reloadButton);

        QPushButton* mergeButton = new QPushButton();
        mergeButton->setText(tr("Merge Changes"));
        mergeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(mergeButton, &QPushButton::clicked, [=] {
            d->currentBackend->load()->then([=](QByteArray currentFile) {
                bool mergeResolutionRequred;
                QString mergeFile = MergeTool::getUnmergedFile(d->textCodec->toUnicode(currentFile), this->toPlainText(), tr("File on Disk"), tr("Currently open file"), &mergeResolutionRequred);

                if (mergeResolutionRequred) {
                    //Open the merge resolution tool
                    MergeTool* tool = new MergeTool(mergeFile, d->hlDef, d->parentWindow);
                    tool->setTitle(tr("Resolve a Save Conflict"));
                    tool->setParent(this);

                    #if THE_LIBS_API_VERSION >= 3 && !defined(Q_OS_MAC)
                        tool->setWindowFlags(Qt::Widget);
                        tPopover* popover = new tPopover(tool);
                        popover->setPopoverWidth(-100 * theLibsGlobal::getDPIScaling());
                        popover->show(this->window());

                        connect(tool, &MergeTool::acceptResolution, [=](QString revisedFile) {
                            this->setPlainText(revisedFile);
                            removeTopPanel(d->onDiskChanged);
                        });
                        connect(tool, &MergeTool::finished, [=] {
                            d->parentWindow->menuBar()->setEnabled(true);
                            popover->dismiss();
                        });
                        connect(popover, &tPopover::dismissed, [=] {
                            tool->reject();
                        });
                    #else
                        tool->setWindowFlag(Qt::Sheet);
                        tool->setModal(Qt::WindowModal);
                        tool->show();
                        d->parentWindow->menuBar()->setEnabled(false);

                        connect(tool, &MergeTool::acceptResolution, [=](QString revisedFile) {
                            this->setPlainText(revisedFile);
                            removeTopPanel(d->onDiskChanged);
                        });
                        connect(tool, &MergeTool::finished, [=] {
                            d->parentWindow->menuBar()->setEnabled(true);
                        });
                    #endif
                } else {
                    //Merge resolution tool not needed
                    this->setPlainText(mergeFile);
                    removeTopPanel(d->onDiskChanged);
                }
            })->error([=](QString err) {

            });
        });
        d->onDiskChanged->addButton(mergeButton);

        connect(d->onDiskChanged, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->onDiskChanged);
        });
    }

    {
        d->onDiskDeleted = new TopNotification();
        d->onDiskDeleted->setTitle(tr("File on disk deleted"));
        d->onDiskDeleted->setText(tr("The file on the disk has been deleted. If you save this file you will recreate the file."));

        connect(d->onDiskDeleted, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->onDiskDeleted);
        });
    }

    {
        d->fileReadError = new TopNotification();
        d->fileReadError->setTitle(tr("Can't open file"));

        connect(d->fileReadError, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->fileReadError);
        });
    }

    {
        d->mixedLineEndings = new TopNotification();
        d->mixedLineEndings->setTitle(tr("Mixed Line Endings detected"));
        d->mixedLineEndings->setText(tr("When saving this file, we'll normalise all the line endings. You can choose which line endings to save as on the status bar."));

        connect(d->mixedLineEndings, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->mixedLineEndings);
        });
    }

    {
        d->decodingProblem = new TopNotification();
        d->decodingProblem->setTitle(tr("Incorrect Text Encoding"));

        QPushButton* selectEncodingButton = new QPushButton();
        selectEncodingButton->setText(tr("Select Encoding"));
        connect(selectEncodingButton, &QPushButton::clicked, this, [=] {
            this->chooseCodec(true);
        });
        d->decodingProblem->addButton(selectEncodingButton);
    }

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, [=](int position) {
        if (d->scrollingLock != nullptr) {
            d->scrollingLock->verticalScrollBar()->setValue(position);
        }
    });

    connect(static_cast<QApplication*>(QApplication::instance()), &QApplication::paletteChanged, this, &TextEditor::reloadSettings);
    reloadSettings();
}

TextEditor::~TextEditor() {
    if (d->button != nullptr) {
        d->button->setVisible(false);
    }
    d->leftMargin->deleteLater();
    delete d;
}

void TextEditor::setMainWindow(MainWindow *mainWindow) {
    d->parentWindow = mainWindow;
}

void TextEditor::setStatusBar(TextStatusBar *statusBar) {
    d->statusBar = statusBar;

    cursorLocationChanged();
    setTextCodec(QTextCodec::codecForName("UTF-8"));
    setHighlighter(highlightRepo->definitionForName("None"));

    reloadSettings();
}

TabButton* TextEditor::getTabButton() {
    return d->button;
}

void TextEditor::setActive(bool active) {
    d->button->setActive(active);
    d->active = active;
}

QString TextEditor::title() {
    if (d->currentBackend == nullptr) {
        return "";
    } else {
        return d->currentBackend->documentTitle();
    }
}

bool TextEditor::isEdited() {
    return d->edited;
}

void TextEditor::openFile(FileBackend *backend) {
    removeTopPanel(d->onDiskChanged);
    removeTopPanel(d->onDiskDeleted);
    removeTopPanel(d->fileReadError);
    removeTopPanel(d->mixedLineEndings);
    removeTopPanel(d->decodingProblem);

    d->cover->setVisible(true);
    d->cover->raise();
    backend->load()->then([=](QByteArray data) {
        loadText(data);

        /*QMap<SyntaxHighlighting*, QString> recommendedBackend;
        for (SyntaxHighlighting* h : plugins->syntaxHighlighters()) {
            QString f = h->highlighterForFilename(backend->url().toString());
            if (f != "") recommendedBackend.insert(h, f);
        }
        if (recommendedBackend.count() != 0) this->setHighlighter(recommendedBackend.firstKey()->makeHighlighter(recommendedBackend.first(), &getSyntaxHighlighterColor));*/

        KSyntaxHighlighting::Definition def = highlightRepo->definitionForFileName(backend->url().toString());
        if (def.isValid()) {
            setHighlighter(def);
        }

        if (d->parentWindow != nullptr) d->parentWindow->updateGit();

        emit backendChanged();
        d->cover->setVisible(false);

        if (backend->url().scheme() == "file") {
            recentFiles->putFile(backend->url().toString());
        }
    })->error([=](QString error) {
        d->fileReadError->setText(error);

        d->fileReadError->clearButtons();

        QPushButton* retryButton = new QPushButton();
        retryButton->setText(tr("Retry"));
        retryButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(retryButton, &QPushButton::clicked, [=] {
            openFile(backend);
        });
        d->fileReadError->addButton(retryButton);

        addTopPanel(d->fileReadError);
        d->cover->setVisible(false);
    });

    d->currentBackend = backend;
    connectBackend();
}

void TextEditor::loadText(QByteArray data) {
    if (d->textCodec == nullptr) {
        setTextCodec(QTextCodec::codecForUtfText(data, QTextCodec::codecForName("UTF-8")));
    }

    QTextDecoder* decoder = d->textCodec->makeDecoder(QTextCodec::ConvertInvalidToNull);
    QString decoded = decoder->toUnicode(data);
    this->setPlainText(decoded);
    d->edited = false;

    if (decoder->hasFailure()) {
        d->decodingProblem->setText(tr("We tried opening this file with the %1 encoding, but it contains invalid characters. If you save the file in the incorrect encoding, you may lose data.").arg(QString(d->textCodec->name())));
        addTopPanel(d->decodingProblem);
    }

    delete decoder;

    //Detect line endings;
    char endings = 0;
    if (data.contains("\r\n"))                                    endings |= 0b001;
    if (QRegularExpression("\\r(?!\\n)").match(data).hasMatch())  endings |= 0b010;
    if (QRegularExpression("(?<!\\r)\\n").match(data).hasMatch()) endings |= 0b100;

    switch (endings) {
        case 0b000: //No line endings
            if (d->statusBar != nullptr) d->statusBar->setLineEndings(-1);
            break;
        case 0b001: //Windows
            if (d->statusBar != nullptr) d->statusBar->setLineEndings(2);
            break;
        case 0b010: //Macintosh
            if (d->statusBar != nullptr) d->statusBar->setLineEndings(1);
            break;
        case 0b100: //UNIX
            if (d->statusBar != nullptr) d->statusBar->setLineEndings(0);
            break;
        default: //Mixed line endings
            if (d->statusBar != nullptr) d->statusBar->setLineEndings(-1);
            addTopPanel(d->mixedLineEndings);
    }

    //Detect VCS conflicts
    if (this->toPlainText().contains("======")) {
        addTopPanel(d->mergeConflictsNotification);
    } else {
        removeTopPanel(d->mergeConflictsNotification);
    }

    emit editedChanged();
}

bool TextEditor::saveFile() {
    if (d->currentBackend == nullptr) {
        return false;
    } else if (d->currentBackend->readOnly()) {
        tMessageBox* messageBox = new tMessageBox(this->window());
        messageBox->setWindowTitle(tr("Read Only File"));
        messageBox->setText(tr("This file is read only. You'll need to save it as a different file."));
        messageBox->setIcon(tMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(tMessageBox::Ok);
        messageBox->setDefaultButton(tMessageBox::Ok);
        messageBox->exec();
        return true;
    } else {
        QByteArray saveData = formatForSaving(this->toPlainText());
        if (!d->textCodec->canEncode(this->toPlainText())) {
            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Encoding Error"));
            messageBox->setText(tr("Some characters used in this file cannot be encoded in the selected encoding. Saving this file will remove any invalid characters and may result in possible data loss."));
            messageBox->setIcon(tMessageBox::Warning);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(tMessageBox::Cancel | tMessageBox::Save);
            messageBox->setDefaultButton(tMessageBox::Ok);
            if (messageBox->exec() == tMessageBox::Cancel) return true;
        }
        d->currentBackend->save(saveData)->then([=] {
            removeTopPanel(d->onDiskChanged);
            removeTopPanel(d->onDiskDeleted);
            removeTopPanel(d->decodingProblem);
            d->edited = false;

            if (d->parentWindow != nullptr) d->parentWindow->updateGit();

            emit backendChanged();
            emit editedChanged();
            d->leftMargin->update();

            if (d->currentBackend->url().scheme() == "file") {
                recentFiles->putFile(d->currentBackend->url().toString());
            }
        })->error([=](QString error) {
            QString text;

            if (error.startsWith("$")) {
                text = error.mid(1);
            } else if (error == "Disk Full") {
                text = tr("There's no more space on this disk.");
            } else if (error == "Permissions") {
                text = tr("You don't have permission to save this file.");
            } else {
                text = tr("Unable to save this file. Check that you have permissions to write to this file and that there's enough space on disk.");
            }

            text.append(tr("\n\nDo not exit theSlate until you've managed to write the file, otherwise you may lose data."));

            tMessageBox* messageBox = new tMessageBox(this->window());
            messageBox->setWindowTitle(tr("Couldn't save the file"));
            messageBox->setText(text);
            messageBox->setIcon(tMessageBox::Critical);
            messageBox->setWindowFlags(Qt::Sheet);
            messageBox->setStandardButtons(tMessageBox::Ok);
            messageBox->setDefaultButton(tMessageBox::Ok);
            messageBox->exec();
        });
        return true;
    }
}

QSyntaxHighlighter* TextEditor::highlighter() {
    return d->hl;
}

void TextEditor::keyPressEvent(QKeyEvent *event) {
    QTextCursor cursor = this->textCursor();
    bool handle = false;

    QString spacingCharacters = d->getIndentCharacters();
    int spaceNum = spacingCharacters.count();
    bool tabSpaces = d->settings.value("behaviour/tabSpaces", true).toBool();

    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        QTextCursor startCursor(this->document());
        startCursor.setPosition(cursor.selectionStart());

        QTextCursor endCursor(this->document());
        endCursor.setPosition(cursor.selectionEnd());

        if (startCursor.blockNumber() == endCursor.blockNumber()) {
            //Insert a tab
            if (event->key() == Qt::Key_Tab) cursor.insertText(spacingCharacters);
        } else {
            startCursor.beginEditBlock();
            if (event->key() == Qt::Key_Backtab) {
                //Outdent each line
                d->forEverySelectedLine(cursor, [=](QTextCursor cursor) {
                    for (int i = 0; i < spaceNum; i++) {
                        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                        if (cursor.selectedText() == spacingCharacters.at(0)) {
                            cursor.removeSelectedText();
                        }
                        cursor.movePosition(QTextCursor::StartOfBlock);
                    }
                });
            } else {
                d->forEverySelectedLine(cursor, [=](QTextCursor cursor) {
                    cursor.insertText(spacingCharacters);
                });
            }
            startCursor.endEditBlock();
        }
        handle = true;
    } else if (event->key() == Qt::Key_Backspace) {
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        QString leftChar = cursor.selectedText();
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString rightChar = cursor.selectedText();
        if ((leftChar == "(" && rightChar == ")") || (leftChar == "[" && rightChar == "]") || (leftChar == "{" && rightChar == "}")) {
            cursor.deleteChar();
            cursor.deletePreviousChar();
            handle = true;
        } else {
            cursor.movePosition(QTextCursor::Left);
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            QString text = cursor.selectedText();
            if (tabSpaces && text.endsWith(spacingCharacters) && text.length() % spaceNum == 0) {
                cursor = this->textCursor();
                for (int i = 0; i < spaceNum; i++) {
                    cursor.deletePreviousChar();
                }
                handle = true;
            }
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        int spaces = 0;
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);

        if (cursor.selectedText() == "{" || cursor.selectedText() == ":" || cursor.selectedText() == "[") {
            spaces = spaceNum;
        }

        QString testCharacter;
        if (tabSpaces) {
            testCharacter = " ";
        } else {
            testCharacter = "\t";
        }
        cursor.select(QTextCursor::LineUnderCursor);
        QString text = cursor.selectedText();
        while (text.startsWith(testCharacter)) {
            spaces++;
            text.remove(0, 1);
        }
        cursor = this->textCursor();
        cursor.insertText("\n");
        for (int i = 0; i < spaces; i++) {
            cursor.insertText(testCharacter);
        }

        if (cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor)) {
            if (cursor.selectedText() == "}") {
                cursor.movePosition(QTextCursor::Left);
                cursor.insertText("\n");
                for (int i = 0; i < spaces - spaceNum; i++) {
                    cursor.insertText(testCharacter);
                    cursor.movePosition(QTextCursor::Left);
                }
                cursor.movePosition(QTextCursor::Left);
            } else if (cursor.selectedText() == "]") {
                    cursor.movePosition(QTextCursor::Left);
                    cursor.insertText("\n");
                    for (int i = 0; i < spaces - spaceNum; i++) {
                        cursor.insertText(testCharacter);
                        cursor.movePosition(QTextCursor::Left);
                    }
                    cursor.movePosition(QTextCursor::Left);
            } else {
                cursor.movePosition(QTextCursor::Left);
            }
        }

        handle = true;
    } else if (event->text() == "}") {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString text = cursor.selectedText();
        if (tabSpaces && text.endsWith(spacingCharacters) && text.length() % spaceNum == 0) {
            cursor = this->textCursor();
            for (int i = 0; i < spaceNum; i++) {
                cursor.deletePreviousChar();
            }
            cursor.insertText("}");
            handle = true;
        }
    } else if (event->text() == "{") {
        cursor.insertText("{");
        cursor.insertText("}");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == "[") {
        cursor.insertText("[");
        cursor.insertText("]");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == "(") {
        cursor.insertText("(");
        cursor.insertText(")");
        cursor.movePosition(QTextCursor::Left);
        handle = true;
    } else if (event->text() == ")") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == ")") {
            cursor = this->textCursor();
            cursor.movePosition(QTextCursor::Right);
            handle = true;
        }
    } else if (event->text() == "]") {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == "]") {
            cursor = this->textCursor();
            cursor.movePosition(QTextCursor::Right);
            handle = true;
        }
    }

    if (handle) {
        this->setTextCursor(cursor);
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

int TextEditor::leftMarginWidth()
{
    int digits = 1;
    int max = qMax(1, this->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().height() + fontMetrics().width(QLatin1Char('9')) * digits;
    space += 9 * theLibsGlobal::getDPIScaling();

    return space;
}

void TextEditor::updateLeftMarginAreaWidth()
{
    d->topPanelWidget->updateGeometry();
    int height = d->topPanelWidget->sizeHint().height();
    d->topPanelWidget->setFixedHeight(height);
    setViewportMargins(leftMarginWidth() - 4 * theLibsGlobal::getDPIScaling(), height, 0, 0);
}

void TextEditor::updateLeftMarginArea(const QRect &rect, int dy)
{
    if (dy) {
        d->leftMargin->scroll(0, dy);
    } else {
        d->leftMargin->update(0, rect.y(), d->leftMargin->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLeftMarginAreaWidth();
    }
}

void TextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    d->leftMargin->setGeometry(QRect(cr.left(), cr.top() + d->topPanelWidget->sizeHint().height(), leftMarginWidth(), cr.height() - 1));

    d->topPanelWidget->setFixedWidth(this->width());

    d->cover->resize(this->size());
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextBlock currentBlock = this->textCursor().block();
    d->highlightedLine = currentBlock.firstLineNumber();

    if (!this->isReadOnly()) {
        QTextCursor cursor = this->textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock);

        bool success = true;
        while (cursor.block() == currentBlock && success) {
            QTextEdit::ExtraSelection selection;

            QColor windowCol = this->palette().color(QPalette::Window);
            selection.format.setBackground(QColor(highlightTheme.editorColor(KSyntaxHighlighting::Theme::CurrentLine)));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setProperty(QTextFormat::UserFormat, "currentHighlight");
            selection.cursor = cursor;
            selection.cursor.clearSelection();
            extraSelections.append(selection);

            success = cursor.movePosition(QTextCursor::Down);
        }
    }

    setExtraSelectionGroup(100, "lineHighlight", extraSelections);
}

void TextEditor::cursorLocationChanged() {
    if (this->textCursor().block().userData() == nullptr) this->textCursor().block().setUserData(new TextEditorBlockData(this));
    highlightCurrentLine();
    if (d->statusBar != nullptr) d->statusBar->setPosition(this->textCursor().blockNumber(), this->textCursor().columnNumber());

    clearExtraSelectionGroup("bracketLocation");
    QTextCursor c(textCursor());
    int startPosition = c.position();
    c.clearSelection();
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QString firstChar = c.selectedText();
    QStringList bracketDetection = {
        "(","{","[",
        ")","}","]"
    };

    bool search = false;
    if (bracketDetection.contains(firstChar)) {
        search = true;
    } else {
        c.setPosition(c.anchor());
        c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 1);
        firstChar = c.selectedText();
        if (bracketDetection.contains(firstChar)) {
            search = true;
        }
    }

    if (search) {
        QTextCursor endCursor(c);
        endCursor.movePosition(QTextCursor::End);

        QTextCursor startCursor(c);
        startCursor.movePosition(QTextCursor::Start);

        QTextCursor firstCursor = QTextCursor(c);
        c.setPosition(c.selectionStart());

        QString searchChar;
        bool stepForwards = true;
        int firstIndex = bracketDetection.indexOf(firstChar);
        if (firstIndex >= bracketDetection.count() / 2) {
            searchChar = bracketDetection.at(firstIndex - bracketDetection.count() / 2);
            stepForwards = false;
        } else {
            searchChar = bracketDetection.at(firstIndex + bracketDetection.count() / 2);
        }

        int currentIndentation = 1;
        int otherPosition = -1;

        while (otherPosition == -1) {
            if (stepForwards) {
                if (c.selectionStart() == endCursor.position()) {
                    otherPosition = -2;
                    continue;
                }
            } else {
                if (c.selectionStart() == startCursor.position()) {
                    otherPosition = -2;
                    continue;
                }
            }

            c.setPosition(c.selectionStart());
            if (!c.movePosition(stepForwards ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter)) {
                otherPosition = -2;
                continue;
            }
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (c.selectedText() == firstChar) {
                currentIndentation++;
            } else if (c.selectedText() == searchChar) {
                currentIndentation--;
                if (currentIndentation == 0) {
                    //Found the other character!
                    otherPosition = c.position();
                }
            }
        }

        if (otherPosition == -2) {
            //Mismatched!
        } else {
            //Found it!
            QTextCharFormat format;
            format.setBackground(QColor(255, 255, 0));
            format.setForeground(QColor(255, 0, 0));

            QTextEdit::ExtraSelection firstSelection;
            firstSelection.cursor = firstCursor;
            firstSelection.format = format;
            firstSelection.format.setProperty(QTextFormat::FullWidthSelection, false);

            QTextEdit::ExtraSelection secondSelection;
            secondSelection.cursor = c;
            secondSelection.format = format;
            secondSelection.format.setProperty(QTextFormat::FullWidthSelection, false);

            setExtraSelectionGroup(900, "bracketLocation", {
                                       firstSelection, secondSelection
                                   });
        }

    }
}

void TextEditor::leftMarginPaintEvent(QPaintEvent *event)
{
    QPainter painter(d->leftMargin);
    painter.fillRect(event->rect(), this->palette().color(QPalette::Window));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        int lineNo = blockNumber + 1;

        painter.setPen(Qt::transparent);
        QColor lineNumberColor = highlightTheme.editorColor(KSyntaxHighlighting::Theme::LineNumbers);
        QColor backgroundColor = highlightTheme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor);

        //Draw current line background if we're not read only
        if (!this->isReadOnly()) {
            if (this->textCursor().block() == block) {
                backgroundColor = highlightTheme.editorColor(KSyntaxHighlighting::Theme::CurrentLine);
                lineNumberColor = highlightTheme.editorColor(KSyntaxHighlighting::Theme::CurrentLineNumber);
            }
        }

        //Draw merge conflict marker
        for (MergeLines mergeBlock : d->mergedLines) {
            for (int i = 0; i < mergeBlock.length; i++) {
                int mergeLine = mergeBlock.startLine + i;
                if (blockNumber == mergeLine) {
                    if (d->mergeDecisions.value(mergeBlock)) {
                        backgroundColor = QColor(0, 150, 0);
                    } else {
                        backgroundColor = QColor(100, 100, 100);
                    }
                }
            }
        }

        //Draw line number and background
        QString number = QString::number(lineNo);
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setBrush(backgroundColor);
            painter.drawRect(0, top, d->leftMargin->width(), bottom - top);

            QFont font = this->font();
            font.setPointSizeF(font.pointSizeF() * 0.8);

            painter.setFont(font);
            painter.setPen(lineNumberColor);
            painter.drawText(0, top, d->leftMargin->width() - 9 * theLibsGlobal::getDPIScaling(), fontMetrics().height(), Qt::AlignRight | Qt::AlignCenter, number);
        }

        TextEditorBlockData* blockData = (TextEditorBlockData*) block.userData();
        if (blockData != nullptr) {
            painter.setPen(Qt::transparent);
            switch (blockData->marginState) {
                case TextEditorBlockData::None:
                    painter.setBrush(Qt::transparent);
                    break;
                case TextEditorBlockData::Edited:
                    painter.setBrush(QColor(200, 0, 0));
                    break;
                case TextEditorBlockData::SavedEdited:
                    painter.setBrush(QColor(0, 200, 0));
                    break;
            }
            painter.drawRect(0, top, 3 * theLibsGlobal::getDPIScaling(), bottom - top);
        }

        if (d->hl->startsFoldingRegion(block)) {

        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
}

void TextEditor::reloadBlockHighlighting() {
    QList<QTextEdit::ExtraSelection> extraSelections = extraSelectionGroup("blockHighlighting");

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid()) {
        int lineNo = blockNumber + 1;
        if (d->brokenLine == lineNo) {
            QTextEdit::ExtraSelection selection;

            selection.format.setBackground(Qt::yellow);
            selection.format.setForeground(Qt::black);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setProperty(QTextFormat::UserFormat, "breakingLine");
            selection.cursor = cursorForPosition(QPoint(blockBoundingGeometry(block).top() + 1, 0));
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
    setExtraSelectionGroup(500, "blockHighlighting", extraSelections);

    d->leftMargin->repaint();
    highlightCurrentLine();
}

QList<QTextEdit::ExtraSelection> TextEditor::extraSelectionGroup(QString extraSelectionGroup) {
    return d->extraSelectionGroups.value(extraSelectionGroup);
}

void TextEditor::setExtraSelectionGroup(int priority, QString extraSelectionGroup, QList<QTextEdit::ExtraSelection> selections) {
    d->extraSelectionGroups.insert(QString::number(priority) + "-" + extraSelectionGroup, selections);
    updateExtraSelections();
}

void TextEditor::clearExtraSelectionGroup(QString extraSelectionGroups) {
    for (QString selection : d->extraSelectionGroups.keys()) {
        if (selection.endsWith("-" + extraSelectionGroups)) {
            d->extraSelectionGroups.remove(selection);
            updateExtraSelections();
            return;
        }
    }
}

void TextEditor::updateExtraSelections() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    for (QList<QTextEdit::ExtraSelection> extraSelectionGroup : d->extraSelectionGroups.values()) {
        extraSelections.append(extraSelectionGroup);
    }

    setExtraSelections(extraSelections);
}

void TextEditor::setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections) {
    d->leftMargin->repaint();
    QPlainTextEdit::setExtraSelections(extraSelections);
}

void TextEditor::openFileFake(QString contents) {
    this->setPlainText(contents);

    d->edited = false;
    emit backendChanged();
    emit editedChanged();
}

void TextEditor::revertFile(QTextCodec* codec) {
    if (d->currentBackend != nullptr) {
        tMessageBox* messageBox = new tMessageBox(this->window());
        messageBox->setWindowTitle(tr("Revert Changes?"));
        messageBox->setText(tr("Do you want to revert all the edits made to this document?"));
        messageBox->setIcon(tMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(tMessageBox::Yes | tMessageBox::No);
        messageBox->setDefaultButton(tMessageBox::Save);
        int button = messageBox->exec();

        if (button == tMessageBox::Yes) {
            if (codec != nullptr) setTextCodec(codec);
            openFile(d->currentBackend);
        }
    }
}

void TextEditor::dragEnterEvent(QDragEnterEvent *event) {
    const QMimeData* data = event->mimeData();
    if (data->hasUrls()) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    } else if (data->hasText()) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        d->cursorBeforeDrop = this->textCursor();
        this->setTextCursor(this->cursorForPosition(event->pos()));
        return;
    }

    event->setDropAction(Qt::IgnoreAction);
}

void TextEditor::dragLeaveEvent(QDragLeaveEvent *event) {
    this->setTextCursor(d->cursorBeforeDrop);
}

void TextEditor::dragMoveEvent(QDragMoveEvent *event) {
    const QMimeData* data = event->mimeData();
    if (data->hasText()) {
        this->setTextCursor(this->cursorForPosition(event->pos()));
    }
}

void TextEditor::dropEvent(QDropEvent *event) {
    const QMimeData* data = event->mimeData();
    if (data->hasUrls()) {
        for (QUrl url : data->urls()) {
            if (url.isLocalFile()) {
                d->parentWindow->newTab(url.toLocalFile());
            }
        }
    } else if (data->hasText()) {
        this->cursorForPosition(event->pos()).insertText(data->text());
    }
}

bool TextEditor::saveFileAskForFilename(bool saveAs) {
    if (d->currentBackend == nullptr || saveAs) {
        bool ok;
        QUrl url = plugins->getLocalFileBackend()->askForUrl(this, &ok);

        if (ok) {
            d->currentBackend = plugins->getLocalFileBackend()->openFromUrl(url);
            connectBackend();
            this->saveFile();
            return true;
        } else {
            return false;
        }
    } else {
        return this->saveFile();
    }
}

void TextEditor::addTopPanel(QWidget* topPanel) {
    d->topPanelLayout->addWidget(topPanel);
    topPanel->setVisible(true);
    updateLeftMarginAreaWidth();

    if (qobject_cast<TopNotification*>(topPanel)) {
        emit primaryTopNotificationChanged(qobject_cast<TopNotification*>(topPanel));
    }
}

void TextEditor::removeTopPanel(QWidget* topPanel) {
    d->topPanelLayout->removeWidget(topPanel);
    topPanel->setVisible(false);
    updateLeftMarginAreaWidth();

    emit primaryTopNotificationChanged(nullptr);
}

void TextEditor::connectBackend() {
    connect(d->currentBackend, &FileBackend::remoteFileEdited, this, [=] {
        addTopPanel(d->onDiskChanged);
    });
    connect(d->currentBackend, &FileBackend::remoteFileRemoved, this, [=] {
        addTopPanel(d->onDiskDeleted);
    });
}

void TextEditor::lockScrolling(TextEditor *other) {
    if (d->scrollingLock != other) {
        d->scrollingLock = other;
        other->lockScrolling(this);
    }
}

void TextEditor::setMergedLines(QList<MergeLines> mergedLines) {
    d->mergedLines = mergedLines;
    for (MergeLines mergeBlock : mergedLines) {
        d->mergeDecisions.insert(mergeBlock, false);
    }
    updateMergedLinesColour();
}

void TextEditor::toggleMergedLines(int line) {
    for (MergeLines mergeBlock : d->mergedLines) {
        for (int i = 0; i < mergeBlock.length; i++) {
            int mergeLine = mergeBlock.startLine + i;
            if (mergeLine == line - 1) {
                bool currentDecision = d->mergeDecisions.value(mergeBlock);
                d->mergeDecisions.insert(mergeBlock, !currentDecision);
                updateMergedLinesColour();
                emit mergeDecision(mergeBlock, !currentDecision);
                d->leftMargin->update();
                return;
            }
        }
    }
}

void TextEditor::updateMergedLinesColour() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    for (MergeLines mergeBlock : d->mergedLines) {
        QTextCursor cur(this->document());
        QBrush lineColor;
        if (d->mergeDecisions.value(mergeBlock)) {
            lineColor = QColor(0, 200, 0, 100);
        } else {
            lineColor = QColor(150, 150, 150, 100);
        }
        //QColor textColor = QColor(0, 0, 0);
        cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, mergeBlock.startLine);

        for (int i = 0; i < mergeBlock.length; i++) {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(lineColor);
            //selection.format.setForeground(textColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setProperty(QTextFormat::UserFormat, "currentHighlight");
            selection.cursor = cur;
            extraSelections.append(selection);
            cur.movePosition(QTextCursor::NextBlock);
        }
    }
    setExtraSelectionGroup(600, "mergeConflictResolution", extraSelections);
}

bool TextEditor::mergedLineIsAccepted(MergeLines mergedLine) {
    return d->mergeDecisions.value(mergedLine);
}

void TextEditor::setHighlighter(KSyntaxHighlighting::Definition hl) {
    if (d->hl != nullptr) {
        d->hl->deleteLater();
    }

    SyntaxHighlighter* highlighter = new SyntaxHighlighter(this);
    if (hl.isValid()) {
        highlighter->setDefinition(hl);
    } else {
        highlighter->setDefinition(highlightRepo->definitionForName("None"));
    }

    highlighter->setTheme(highlightTheme);
    highlighter->setDocument(this->document());

    if (d->statusBar != nullptr) d->statusBar->setHighlighting(hl);

    d->hl = highlighter;
    d->hlDef = hl;
}

void TextEditor::reloadSettings() {
    d->settings.sync();
    QFont f;
    if (d->settings.value("font/useSystem", true).toBool()) {
        f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    } else {
        f = QFont(d->settings.value("font/textFontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString(), d->settings.value("font/textFontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());
    }
    this->setFont(f);
    this->setTabStopDistance(QFontMetrics(f).width(" ") * d->settings.value("behaviour/tabWidth", 4).toInt());

    //Set up palette
    QPalette pal = QApplication::palette(this);
    pal.setColor(QPalette::Window, highlightTheme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
    pal.setColor(QPalette::Base, highlightTheme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
    //pal.setColor(QPalette::WindowText, getSyntaxHighlighterColor("editor/fg"));
    //pal.setColor(QPalette::Text, getSyntaxHighlighterColor("editor/fg"));
    this->setPalette(pal);

    if (d->statusBar != nullptr) d->statusBar->setSpacing(d->settings.value("behaviour/tabSpaces", true).toBool(), d->settings.value("behaviour/tabSpaceNumber", 4).toInt());

    if (d->settings.value("behaviour/wrapText", false).toBool()) {
        this->setLineWrapMode(WidgetWidth);
    } else {
        this->setLineWrapMode(NoWrap);
    }
}

QUrl TextEditor::fileUrl() {
    if (d->currentBackend != nullptr) {
        return d->currentBackend->url();
    } else {
        return QUrl();
    }
}

QByteArray TextEditor::formatForSaving(QString text) {
    if (d->textCodec == nullptr) {
        setTextCodec(QTextCodec::codecForName("UTF-8"));
    }

    removeTopPanel(d->mixedLineEndings);
    QByteArray a = d->textCodec->fromUnicode(text);

    switch (d->statusBar->lineEndings()) {
        case 0: //Unix
            //do nothing
            break;
        case 1: //Macintosh
            a.replace("\n", "\r");
            break;
        case 2: //Windows
            a.replace("\n", "\r\n");
    }

    return a;
}

void TextEditor::setTextCodec(QTextCodec* codec) {
    d->textCodec = codec;
    if (d->statusBar != nullptr) d->statusBar->setEncoding(codec->name());
}

void TextEditor::commentSelectedText() {
    QTextCursor cursor = this->textCursor();

    if (d->hlDef.multiLineCommentMarker().first == "" && d->hlDef.singleLineCommentMarker() == "") {
        //Comments aren't supported in this language
        tToast* toast = new tToast();
        toast->setTitle(tr("Comments Not Supported"));
        toast->setText(tr("Comments are not supported in this language. Select a different language mode to add comments."));
        toast->show(this->window());
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        return;
    }

    //Find any available comments and remove them
    QTextCursor startCursor(this->document());
    startCursor.setPosition(cursor.selectionStart());

    QTextCursor endCursor(this->document());
    endCursor.setPosition(cursor.selectionEnd());

    if (d->hlDef.multiLineCommentMarker().first != "") {
        //Check to see if we've got a multi-line comment
        startCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->hlDef.multiLineCommentMarker().first.count());
        endCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, d->hlDef.multiLineCommentMarker().second.count());

        if (startCursor.selectedText() == d->hlDef.multiLineCommentMarker().first && endCursor.selectedText() == d->hlDef.multiLineCommentMarker().second) {
            //Uncomment both
            startCursor.beginEditBlock();
            startCursor.removeSelectedText();
            startCursor.setPosition(endCursor.selectionStart());
            startCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->hlDef.multiLineCommentMarker().second.count());
            startCursor.removeSelectedText();
            startCursor.endEditBlock();
            return;
        }
    }

    if (d->hlDef.singleLineCommentMarker() != "") {
        QSharedPointer<bool> didMakeChages(new bool(false));
        d->forEverySelectedLine(cursor, [=, &didMakeChages](QTextCursor cursor) {
            //Check to see if this line has a comment
            QChar indentCharacter = d->getIndentCharacters().at(0);
            if (d->hlDef.singleLineCommentPosition() == KSyntaxHighlighting::CommentPosition::AfterWhitespace) {
                do {
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                } while (cursor.selectedText().at(cursor.selectedText().length() - 1) == indentCharacter);
                cursor.setPosition(cursor.selectionEnd());
            }

            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->hlDef.singleLineCommentMarker().count() + 1);
            if (cursor.selectedText() == d->hlDef.singleLineCommentMarker() + " ") {
                //Remove the comment marker
                cursor.removeSelectedText();
                didMakeChages.reset(new bool(true));
                return;
            }

            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == d->hlDef.singleLineCommentMarker()) {
                //Remove the comment marker
                cursor.removeSelectedText();
                didMakeChages.reset(new bool(true));
                return;
            }
        });

        if (*didMakeChages.data()) {
            return;
        }
    }

    //If we've gotten to here, we've found no comments
    //Proceed to comment

    //Determine the type of commenting to use
    int commentType = 0;

    if (d->hlDef.singleLineCommentMarker() != "" && d->hlDef.multiLineCommentMarker().first != "") {
        if (cursor.hasSelection()) {
            QTextCursor startCursor(this->document());
            startCursor.setPosition(cursor.selectionStart());

            QTextCursor endCursor(this->document());
            endCursor.setPosition(cursor.selectionEnd());

            QTextCursor startOfLineCursor(startCursor);
            startOfLineCursor.movePosition(QTextCursor::StartOfBlock);

            QTextCursor endOfLineCursor(endCursor);
            endOfLineCursor.movePosition(QTextCursor::EndOfBlock);

            if (startOfLineCursor.position() == startCursor.position() && endOfLineCursor.position() == endCursor.position()) {
                commentType = 1; //Single line comment
            } else {
                commentType = 2; //Multi line comment
            }
        } else {
            commentType = 1; //Single line comment
        }
    } else if (d->hlDef.singleLineCommentMarker() != "") {
        commentType = 1; //Single line comment
    } else {
        commentType = 3; //Multi line comment covering whole line
    }

    if (commentType == 1) {
        QChar indentCharacter = d->getIndentCharacters().at(0);
        d->forEverySelectedLine(cursor, [=](QTextCursor cursor) {
            if (d->hlDef.singleLineCommentPosition() == KSyntaxHighlighting::CommentPosition::AfterWhitespace) {
                do {
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                } while (cursor.selectedText().at(cursor.selectedText().length() - 1) == indentCharacter);
                cursor.setPosition(cursor.selectionEnd());
            }

            cursor.insertText(d->hlDef.singleLineCommentMarker() + " ");
        });
    } else if (commentType == 2) {
        QTextCursor startCursor(this->document());
        startCursor.setPosition(cursor.selectionStart());
        int startPosition = cursor.selectionStart();

        startCursor.beginEditBlock();
        startCursor.insertText(d->hlDef.multiLineCommentMarker().first + " ");
        startCursor.setPosition(cursor.selectionEnd());
        startCursor.insertText(" " + d->hlDef.multiLineCommentMarker().second);
        startCursor.endEditBlock();

        cursor.setPosition(startPosition);
        cursor.setPosition(startCursor.selectionEnd(), QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
    } else if (commentType == 3) {
        QTextCursor startCursor(this->document());
        startCursor.setPosition(cursor.selectionStart());
        startCursor.movePosition(QTextCursor::StartOfLine);
        int startPosition = cursor.selectionStart();

        startCursor.beginEditBlock();
        startCursor.insertText(d->hlDef.multiLineCommentMarker().first + " ");
        startCursor.movePosition(QTextCursor::EndOfLine);
        startCursor.insertText(" " + d->hlDef.multiLineCommentMarker().second);
        startCursor.endEditBlock();

        cursor.setPosition(startPosition);
        cursor.setPosition(startCursor.selectionEnd(), QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
    }
}

void TextEditor::chooseHighlighter() {
    QMimeDatabase db;
    QList<SelectListItem> items;

    //Load Syntax Highlighters
    for (KSyntaxHighlighting::Definition d : highlightRepo->definitions()) {
        if (d.name() == "None") continue;

        SelectListItem item(d.translatedName(), d.name());
        items.append(item);
    }

    std::sort(items.begin(), items.end(), [](const SelectListItem &a, const SelectListItem &b) {
        if (a.text.localeAwareCompare(b.text) < 0) {
            return true;
        } else {
            return false;
        }
    });

    items.prepend(SelectListItem(tr("No Highlighting"), "None"));

    SelectListDialog* dialog = new SelectListDialog();
    dialog->setTitle(tr("Select Highlighting"));
    dialog->setText(tr("What type of code is this file?"));
    dialog->setItems(items);

    tPopover* popover = new tPopover(dialog);
    popover->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    connect(dialog, &SelectListDialog::rejected, popover, &tPopover::dismiss);
    connect(dialog, &SelectListDialog::accepted, this, [=](QVariant highlighting) {
        this->setHighlighter(highlightRepo->definitionForName(highlighting.toString()));
        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, dialog, &SelectListDialog::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this->window());
}

void TextEditor::chooseCodec(bool reload) {
    QMimeDatabase db;
    QList<SelectListItem> items;

    //Load encodings
    for (QByteArray codec : QTextCodec::availableCodecs()) {
        SelectListItem item(codec, codec);
        items.append(item);
    }

    std::sort(items.begin(), items.end(), [](const SelectListItem &a, const SelectListItem &b) {
        if (a.text.localeAwareCompare(b.text) < 0) {
            return true;
        } else {
            return false;
        }
    });

    SelectListDialog* dialog = new SelectListDialog();
    dialog->setTitle(tr("Select Encoding"));
    dialog->setText(tr("What file encoding do you want to use?"));
    dialog->setItems(items);

    tPopover* popover = new tPopover(dialog);
    popover->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    connect(dialog, &SelectListDialog::rejected, popover, &tPopover::dismiss);
    connect(dialog, &SelectListDialog::accepted, this, [=](QVariant codec) {
        QTimer::singleShot(0, [=] {
            if (reload) {
                revertFile(QTextCodec::codecForName(codec.toByteArray()));
            } else {
                setTextCodec(QTextCodec::codecForName(codec.toByteArray()));
            }
        });
        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, dialog, &SelectListDialog::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this->window());
}

void TextEditor::gotoLine() {
    bool ok;
    int line = QInputDialog::getInt(this, tr("Go To Line"), tr("What line do you want to go to?"), this->textCursor().blockNumber() + 1, 1, this->document()->blockCount(), 1, &ok);
    if (ok) {
        line--;
        this->setTextCursor(QTextCursor(this->document()->findBlockByLineNumber(line)));
    }
}

void TextEditor::setSelectedTextCasing(Casing casing) {
    QTextCursor cursor = this->textCursor();
    QString text = cursor.selectedText();
    switch (casing) {
        case Uppercase:
            text = text.toUpper();
            break;
        case Lowercase:
            text = text.toLower();
            break;
        case Titlecase: {
            QStringList parts = text.split(" ");
            for (int i = 0; i < parts.count(); i++) {
                QString part = parts.at(i);
                if (!part.isEmpty()) {
                    part = part.toLower();
                    part = part.replace(0, 1, part.at(0).toUpper());
                    parts.replace(i, part);
                }
            }
            text = parts.join(" ");
        }
    }

    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    cursor.beginEditBlock();
    cursor.insertText(text);
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.endEditBlock();
    this->setTextCursor(cursor);
}

TextEditorLeftMargin::TextEditorLeftMargin(TextEditor* editor) : QWidget(editor) {
    this->editor = editor;
    this->setCursor(Qt::ArrowCursor);
}

QSize TextEditorLeftMargin::sizeHint() const {
    return QSize(editor->leftMarginWidth(), 0);
}

void TextEditorLeftMargin::paintEvent(QPaintEvent* event) {
    editor->leftMarginPaintEvent(event);
}

void TextEditorLeftMargin::mousePressEvent(QMouseEvent* event)  {
    int lineNo = editor->cursorForPosition(event->pos()).block().firstLineNumber() + 1;
    editor->toggleMergedLines(lineNo);
}
