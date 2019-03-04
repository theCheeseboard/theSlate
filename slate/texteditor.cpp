#include "texteditor.h"

#include <QStyle>
#include <tmessagebox.h>
#include <QMimeData>
#include <QScrollBar>
#include <QMenuBar>
#include <QSignalBlocker>
#include <tcircularspinner.h>
#include "the-libs_global.h"
#include "mainwindow.h"
#include "plugins/pluginmanager.h"
#include "managers/recentfilesmanager.h"

#include <Repository>
#include <SyntaxHighlighter>
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
        KSyntaxHighlighting::SyntaxHighlighter* hl = nullptr;
        KSyntaxHighlighting::Definition hlDef;
        MainWindow* parentWindow;

        QTextCodec* textCodec = nullptr;

        QTextCursor cursorBeforeDrop;

        TextEditorLeftMargin *leftMargin = nullptr;
        int brokenLine = -1;

        FindReplace* findReplaceWidget;
        QMap<QString, QList<QTextEdit::ExtraSelection>> extraSelectionGroups;

        QList<QWidget*> topPanels;
        QWidget* topPanelWidget;
        QBoxLayout* topPanelLayout;

        TopNotification *mergeConflictsNotification, *onDiskChanged, *fileReadError, *onDiskDeleted, *mixedLineEndings;

        TextEditor* scrollingLock = nullptr;

        int highlightedLine = -1;
        QList<MergeLines> mergedLines;

        QSettings settings;
        QMap<MergeLines, bool> mergeDecisions;

        QWidget* cover;

        int currentLineEndings = -2;

        FileBackend* currentBackend = nullptr;
};


class TextEditorBlockData : public QTextBlockUserData {
    public:
        enum MarginState {
            None,
            Edited,
            SavedEdited
        };

        TextEditorBlockData(TextEditor* parent) {
            editedConnection = QObject::connect(parent, &TextEditor::editedChanged, [=] {
                if (!parent->isEdited() && this->marginState == Edited) this->marginState = SavedEdited;
            });
        }
        ~TextEditorBlockData() {
            QObject::disconnect(editedConnection);
        }

        MarginState marginState = None;

    private:
        QMetaObject::Connection editedConnection;
};

TextEditor::TextEditor(MainWindow *parent) : QPlainTextEdit(parent)
{
    d = new TextEditorPrivate();
    this->setLineWrapMode(NoWrap);

    QFont normalFont = this->font();
    d->parentWindow = parent;

    d->button = new TabButton(this);
    connect(d->button, &TabButton::destroyed, [=] {
        d->button = nullptr;
    });
    d->button->setText(tr("New Document"));

    connect(this, &TextEditor::textChanged, [=] {
        if (d->firstEdit) {
            d->firstEdit = false;
        } else {
            //if (this->textCursor().block().userData() == nullptr) this->textCursor().block().setUserData(new TextEditorBlockData(this));
            //((TextEditorBlockData*) this->textCursor().block().userData())->marginState = TextEditorBlockData::Edited;
            //d->leftMargin->update();
            d->edited = true;
            emit editedChanged();
        }
    });
    connect(this, &TextEditor::backendChanged, [=] {
        d->button->setText(this->title());
        emit titleChanged(this->title());
    });

    d->leftMargin = new TextEditorLeftMargin(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLeftMarginAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLeftMarginArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorLocationChanged()));

    d->leftMargin->setVisible(true);
    highlightCurrentLine();

    d->findReplaceWidget = new FindReplace(this);
    d->findReplaceWidget->setFont(normalFont);
    d->findReplaceWidget->setFixedWidth(500 * theLibsGlobal::getDPIScaling());
    d->findReplaceWidget->setFixedHeight(d->findReplaceWidget->sizeHint().height());
    d->findReplaceWidget->hide();
    //findReplaceWidget->show();

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
        d->mixedLineEndings->setText(tr("If you save this file, we'll change all the line endings to your configuration in Settings."));

        connect(d->mixedLineEndings, &TopNotification::closeNotification, [=] {
            removeTopPanel(d->mixedLineEndings);
        });
    }

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, [=](int position) {
        if (d->scrollingLock != nullptr) {
            d->scrollingLock->verticalScrollBar()->setValue(position);
        }
    });

    connect((QApplication*) QApplication::instance(), &QApplication::paletteChanged, this, &TextEditor::reloadSettings);
    reloadSettings();
}

TextEditor::~TextEditor() {
    if (d->button != nullptr) {
        d->button->setVisible(false);
    }
    delete d;
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

        if (git != nullptr) {
            git->deleteLater();
            git = nullptr;
        }
        if (d->currentBackend->url().isLocalFile()) {
            git = new GitIntegration(QFileInfo(d->currentBackend->url().toLocalFile()).absoluteDir().path());
            connect(git, SIGNAL(reloadStatusNeeded()), d->parentWindow, SLOT(updateGit()));
        }
        d->parentWindow->updateGit();

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
        d->textCodec = QTextCodec::codecForUtfText(data, QTextCodec::codecForName("UTF-8"));
    }

    this->setPlainText(d->textCodec->toUnicode(data));
    d->edited = false;

    //Detect line endings;
    char endings = 0;
    if (data.contains("\r\n"))                                    endings |= 0b001;
    if (QRegularExpression("(?<!\\r)\\n").match(data).hasMatch()) endings |= 0b010;
    if (QRegularExpression("\\r(?!\\n)").match(data).hasMatch())  endings |= 0b100;

    switch (endings) {
        case 0b000: //No line endings
            d->currentLineEndings = -2;
            break;
        case 0b001: //Windows
            d->currentLineEndings = 2;
            break;
        case 0b010: //Macintosh
            d->currentLineEndings = 1;
            break;
        case 0b100: //UNIX
            d->currentLineEndings = 0;
            break;
        default: //Mixed line endings
            d->currentLineEndings = -1;
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
            d->edited = false;

            if (git != nullptr) {
                git->deleteLater();
                git = nullptr;
            }
            if (d->currentBackend->url().isLocalFile()) {
                git = new GitIntegration(QFileInfo(d->currentBackend->url().toLocalFile()).absoluteDir().path());
                connect(git, SIGNAL(reloadStatusNeeded()), d->parentWindow, SLOT(updateGit()));
            }
            d->parentWindow->updateGit();

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

    QString spacingCharacters;
    bool tabSpaces = d->settings.value("behaviour/tabSpaces", true).toBool();
    int spaceNum;
    if (tabSpaces) {
        spaceNum = d->settings.value("behaviour/tabSpaceNumber", 4).toInt();
        spacingCharacters = QString().fill(' ', spaceNum);
    } else {
        spaceNum = 1;
        spacingCharacters = "\t";
    }

    if (event->key() == Qt::Key_Tab) {
        cursor.insertText(spacingCharacters);
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
    /*} else if (event->text() == "\"" && highlighter()->currentCodeType() != SyntaxHighlighter::none) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString right = cursor.selectedText();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
        QString left = cursor.selectedText();
        if (left != "\\") {
            if (right == "\"") {
                cursor = this->textCursor();
                cursor.movePosition(QTextCursor::Right);
            } else {
                cursor = this->textCursor();
                cursor.insertText("\"");
                cursor.insertText("\"");
                cursor.movePosition(QTextCursor::Left);
            }
            handle = true;
        }
    } else if (event->text() == "'" && highlighter()->currentCodeType() != SyntaxHighlighter::none) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString right = cursor.selectedText();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
        QString left = cursor.selectedText();
        if (left != "\\") {
            if (right == "'") {
                cursor = this->textCursor();
                cursor.movePosition(QTextCursor::Right);
            } else {
                cursor = this->textCursor();
                cursor.insertText("'");
                cursor.insertText("'");
                cursor.movePosition(QTextCursor::Left);
            }
            handle = true;
        }*/
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

    return space;
}

void TextEditor::updateLeftMarginAreaWidth()
{
    d->topPanelWidget->updateGeometry();
    int height = d->topPanelWidget->sizeHint().height();
    d->topPanelWidget->setFixedHeight(height);
    setViewportMargins(leftMarginWidth(), height, 0, 0);
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
    d->leftMargin->setGeometry(QRect(cr.left(), cr.top() + d->topPanelWidget->sizeHint().height(), leftMarginWidth(), cr.height()));

    d->findReplaceWidget->setFixedHeight(d->findReplaceWidget->sizeHint().height());
    d->findReplaceWidget->move(this->width() - d->findReplaceWidget->width() - 9 - QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent), 9);

    d->topPanelWidget->setFixedWidth(this->width());

    d->cover->resize(this->size());
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    d->highlightedLine = textCursor().block().firstLineNumber();

    if (!this->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor windowCol = this->palette().color(QPalette::Window);
        QColor lineCol;
        if ((windowCol.red() + windowCol.green() + windowCol.blue()) / 3 > 127) {
            lineCol = QColor(0, 0, 0, 25);
        } else {
            lineCol = QColor(255, 255, 255, 25);
        }

        selection.format.setBackground(lineCol);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.format.setProperty(QTextFormat::UserFormat, "currentHighlight");
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelectionGroup("lineHighlight", extraSelections);
}

void TextEditor::cursorLocationChanged() {
    //if (this->textCursor().block().userData() == nullptr) this->textCursor().block().setUserData(new TextEditorBlockData(this));
    highlightCurrentLine();

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

            setExtraSelectionGroup("bracketLocation", {
                                       firstSelection, secondSelection
                                   });
        }

    }
}

void TextEditor::leftMarginPaintEvent(QPaintEvent *event)
{
    QPainter painter(d->leftMargin);
    painter.fillRect(event->rect(), this->palette().color(QPalette::Window).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        int lineNo = blockNumber + 1;

        for (MergeLines mergeBlock : d->mergedLines) {
            for (int i = 0; i < mergeBlock.length; i++) {
                int mergeLine = mergeBlock.startLine + i;
                if (blockNumber == mergeLine) {
                    painter.setPen(Qt::transparent);
                    if (d->mergeDecisions.value(mergeBlock)) {
                        painter.setBrush(QColor(0, 150, 0));
                    } else {
                        painter.setBrush(QColor(100, 100, 100));
                    }
                    painter.drawRect(0, top, d->leftMargin->width(), bottom - top);
                }
            }
        }

        QString number = QString::number(lineNo);
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(this->palette().color(QPalette::WindowText));
            painter.drawText(0, top, d->leftMargin->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        //TextEditorBlockData* blockData = (TextEditorBlockData*) block.userData();
        /*if (blockData != nullptr) {
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
        }*/

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
    //setExtraSelections(extraSelections);
    setExtraSelectionGroup("blockHighlighting", extraSelections);

    d->leftMargin->repaint();
    highlightCurrentLine();
}

QList<QTextEdit::ExtraSelection> TextEditor::extraSelectionGroup(QString extraSelectionGroup) {
    return d->extraSelectionGroups.value(extraSelectionGroup);
}

void TextEditor::setExtraSelectionGroup(QString extraSelectionGroup, QList<QTextEdit::ExtraSelection> selections) {
    d->extraSelectionGroups.insert(extraSelectionGroup, selections);
    updateExtraSelections();
}

void TextEditor::clearExtraSelectionGroup(QString extraSelectionGroups) {
    if (d->extraSelectionGroups.contains(extraSelectionGroups)) {
        d->extraSelectionGroups.remove(extraSelectionGroups);
    }
    updateExtraSelections();
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

void TextEditor::toggleFindReplace() {
    if (d->findReplaceWidget->isVisible()) {
        d->findReplaceWidget->setVisible(false);
    } else {
        d->findReplaceWidget->setVisible(true);
        d->findReplaceWidget->setFocus();
    }
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
            if (codec != nullptr) d->textCodec = codec;
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
    connect(d->currentBackend, &FileBackend::remoteFileEdited, [=] {
        addTopPanel(d->onDiskChanged);
    });
    connect(d->currentBackend, &FileBackend::remoteFileRemoved, [=] {
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
    setExtraSelectionGroup("mergeConflictResolution", extraSelections);
}

bool TextEditor::mergedLineIsAccepted(MergeLines mergedLine) {
    return d->mergeDecisions.value(mergedLine);
}

void TextEditor::setHighlighter(KSyntaxHighlighting::Definition hl) {
    if (d->hl != nullptr) {
        d->hl->deleteLater();
    }

    KSyntaxHighlighting::SyntaxHighlighter* highlighter = new KSyntaxHighlighting::SyntaxHighlighter(this);
    if (hl.isValid()) {
        highlighter->setDefinition(hl);
    } else {
        highlighter->setDefinition(highlightRepo->definitionForName("None"));
    }

    highlighter->setTheme(highlightTheme);
    highlighter->setDocument(this->document());

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
        d->textCodec = QTextCodec::codecForName("UTF-8");
    }

    removeTopPanel(d->mixedLineEndings);
    QByteArray a = d->textCodec->fromUnicode(text);
    int lineEndingsToSaveAs;

    if (d->currentLineEndings < 0) { //Use settings
        lineEndingsToSaveAs = d->settings.value("behaviour/endOfLine", THESLATE_END_OF_LINE).toInt();
    } else { //Use detected line endings
        lineEndingsToSaveAs = d->currentLineEndings;
    }

    switch (lineEndingsToSaveAs) {
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
