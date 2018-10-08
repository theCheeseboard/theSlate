#include "texteditor.h"

#include <QStyle>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QMenuBar>
#include "the-libs_global.h"
#include "mainwindow.h"

TextEditor::TextEditor(MainWindow *parent) : QPlainTextEdit(parent)
{
    this->setLineWrapMode(NoWrap);
    QFont normalFont = this->font();
    this->parentWindow = parent;

    button = new TabButton(this);
    button->setText(tr("New Document"));

    connect(this, &TextEditor::textChanged, [=] {
        if (firstEdit) {
            firstEdit = false;
        } else {
            edited = true;
            emit editedChanged();
        }
    });
    connect(this, &TextEditor::fileNameChanged, [=] {
        button->setText(QFileInfo(this->filename()).fileName());
        emit titleChanged(QFileInfo(this->filename()).fileName());
    });

    leftMargin = new TextEditorLeftMargin(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLeftMarginAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLeftMarginArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    leftMargin->setVisible(true);
    highlightCurrentLine();

    findReplaceWidget = new FindReplace(this);
    findReplaceWidget->setFont(normalFont);
    findReplaceWidget->setFixedWidth(500 * theLibsGlobal::getDPIScaling());
    findReplaceWidget->setFixedHeight(findReplaceWidget->sizeHint().height());
    findReplaceWidget->hide();
    //findReplaceWidget->show();

    topPanelWidget = new QWidget(this);
    topPanelWidget->move(0, 0);
    topPanelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    topPanelWidget->setFixedWidth(this->width());
    topPanelLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    topPanelLayout->setContentsMargins(0, 1, 0, 0);
    topPanelWidget->setLayout(topPanelLayout);

    {
        mergeConflictsNotification = new TopNotification();
        mergeConflictsNotification->setTitle("Merge Conflicts");
        mergeConflictsNotification->setText(tr("Merge Conflicts were found in this file"));

        QPushButton* fixMergeButton = new QPushButton();
        fixMergeButton->setText(tr("Resolve Merge Conflicts"));
        connect(fixMergeButton, &QPushButton::clicked, [=] {
            //Open the merge resolution tool
            MergeTool* tool = new MergeTool(this->toPlainText(), parentWindow);
            tool->setParent(this);
            tool->setWindowFlag(Qt::Sheet);
            tool->setModal(Qt::WindowModal);
            tool->show();
            parentWindow->menuBar()->setEnabled(false);

            connect(tool, &MergeTool::acceptResolution, [=](QString revisedFile) {
                this->setPlainText(revisedFile);
                removeTopPanel(mergeConflictsNotification);
            });
            connect(tool, &MergeTool::finished, [=] {
                parentWindow->menuBar()->setEnabled(true);
            });
        });;
        mergeConflictsNotification->addButton(fixMergeButton);
    }

    {
        onDiskChanged = new TopNotification();
        onDiskChanged->setTitle(tr("File on disk changed"));
        onDiskChanged->setText(tr("The file on the disk has changed. If you save this file you will lose the changes on disk."));

        QPushButton* reloadButton = new QPushButton();
        reloadButton->setText(tr("Reload File"));
        connect(reloadButton, SIGNAL(clicked(bool)), this, SLOT(revertFile()));
        onDiskChanged->addButton(reloadButton);

        QPushButton* mergeButton = new QPushButton();
        mergeButton->setText(tr("Merge File"));
        onDiskChanged->addButton(mergeButton);
    }

    {
        fileReadError = new TopNotification();
        fileReadError->setTitle(tr("Can't open file"));
    }

    fileWatcher = new QFileSystemWatcher();
    connect(fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileOnDiskChanged()));

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, [=](int position) {
        if (scrollingLock != nullptr) {
            scrollingLock->verticalScrollBar()->setValue(position);
        }
    });

    reloadSettings();
}

TextEditor::~TextEditor() {
    if (button != nullptr) {
        button->setVisible(false);
    }
}

TabButton* TextEditor::getTabButton() {
    return button;
}

void TextEditor::setActive(bool active) {
    button->setActive(active);
    this->active = active;
}

QString TextEditor::filename() {
    return fn;
}

bool TextEditor::isEdited() {
    return edited;
}

void TextEditor::openFile(QString file) {
    removeTopPanel(onDiskChanged);
    removeTopPanel(fileReadError);

    QFile f(file);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QFileInfo info(f);
        if (!info.permission(QFile::ReadUser)) {
            fileReadError->setText(tr("You don't have the appropriate permissions to open %1.").arg(info.fileName()));
        } else if (info.isDir()) {
            fileReadError->setText(tr("%1 is a folder.").arg(info.fileName()));
        } else {
            fileReadError->setText(tr("Can't open %1.").arg(info.fileName()));
        }

        fileReadError->clearButtons();

        QPushButton* retryButton = new QPushButton();
        retryButton->setText(tr("Retry"));
        connect(retryButton, &QPushButton::clicked, [=] {
            openFile(file);
        });
        fileReadError->addButton(retryButton);

        addTopPanel(fileReadError);
        return;
    }
    this->setPlainText(f.readAll());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();

    QFileInfo fileInfo(file);
    /*if (fileInfo.suffix() == "cpp") { //C++ File
        hl->setCodeType(SyntaxHighlighter::cpp);
    } else if (fileInfo.suffix() == "py") { //Python File
        hl->setCodeType(SyntaxHighlighter::py);
    } else if (fileInfo.suffix() == "js") { //Javascript File
        hl->setCodeType(SyntaxHighlighter::js);
    } else if (fileInfo.suffix() == "json") { //JSON File
        hl->setCodeType(SyntaxHighlighter::json);
    } else if (fileInfo.suffix() == "tslprj") { //theSlate Project File
        hl->setCodeType(SyntaxHighlighter::json);
    }*/

    if (git != nullptr) {
        git->deleteLater();
    }
    fileWatcher->removePaths(fileWatcher->files());
    fileWatcher->addPath(file);
    git = new GitIntegration(fileInfo.absoluteDir().path());
    connect(git, SIGNAL(reloadStatusNeeded()), parentWindow, SLOT(updateGit()));

    if (this->toPlainText().contains("======")) {
        addTopPanel(mergeConflictsNotification);
    } else {
        removeTopPanel(mergeConflictsNotification);
    }
}

bool TextEditor::saveFile(QString file) {
    fileWatcher->blockSignals(true);

    QFile f(file);
    if (!f.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox* messageBox = new QMessageBox(this->window());

        QString titleText = tr("Unable to save file");
        QString text = tr("We couldn't save the file.");
        QString informativeText = tr("Here are a few things to check:\n- Ensure that enough disk space is available\n- Ensure that you have write permissions on the file that you want to save to.");

        #ifdef Q_OS_MAC
            messageBox->setText(titleText);
            messageBox->setInformativeText(text);
            messageBox->setDetailedText(informativeText);
        #else
        messageBox->setWindowTitle(titleText);
        messageBox->setText(text);
        messageBox->setInformativeText(informativeText);
        #endif
        messageBox->setIcon(QMessageBox::Critical);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(QMessageBox::Ok);
        messageBox->setDefaultButton(QMessageBox::Ok);
        messageBox->exec();

        messageBox->deleteLater();

        fileWatcher->blockSignals(false);
        return false;
    }
    f.write(this->toPlainText().toUtf8());
    f.close();

    removeTopPanel(onDiskChanged);

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();

    if (git != nullptr) {
        git->deleteLater();
    }
    fileWatcher->removePaths(fileWatcher->files());
    fileWatcher->addPath(file);
    git = new GitIntegration(QFileInfo(file).absoluteDir().path());
    connect(git, SIGNAL(reloadStatusNeeded()), parentWindow, SLOT(updateGit()));

    fileWatcher->blockSignals(false);
    return true;
}

bool TextEditor::saveFile() {
    if (this->filename() == "") {
        return false;
    } else {
        return saveFile(this->filename());
    }
}

QSyntaxHighlighter* TextEditor::highlighter() {
    return hl;
}

void TextEditor::keyPressEvent(QKeyEvent *event) {
    QTextCursor cursor = this->textCursor();
    bool handle = false;
    if (event->key() == Qt::Key_Tab) {
        cursor.insertText("    ");
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
            if (text.endsWith("    ") && text.length() % 4 == 0) {
                cursor = this->textCursor();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                cursor.deletePreviousChar();
                handle = true;
            }
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        int spaces = 0;
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == "{") {
            spaces = 4;
        }
        if (cursor.selectedText() == ":") {
            spaces = 4;
        }
        if (cursor.selectedText() == "[") {
            spaces = 4;
        }

        cursor.select(QTextCursor::LineUnderCursor);
        QString text = cursor.selectedText();
        while (text.startsWith(" ")) {
            spaces++;
            text.remove(0, 1);
        }
        cursor = this->textCursor();
        cursor.insertText("\n");
        for (int i = 0; i < spaces; i++) {
            cursor.insertText(" ");
        }

        if (cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor)) {
            if (cursor.selectedText() == "}") {
                cursor.movePosition(QTextCursor::Left);
                cursor.insertText("\n");
                for (int i = 0; i < spaces - 4; i++) {
                    cursor.insertText(" ");
                    cursor.movePosition(QTextCursor::Left);
                }
                cursor.movePosition(QTextCursor::Left);
            } else if (cursor.selectedText() == "]") {
                    cursor.movePosition(QTextCursor::Left);
                    cursor.insertText("\n");
                    for (int i = 0; i < spaces - 4; i++) {
                        cursor.insertText(" ");
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
        if (text.endsWith("    ") && text.length() % 4 == 0) {
            cursor = this->textCursor();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
            cursor.deletePreviousChar();
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
    topPanelWidget->updateGeometry();
    int height = topPanelWidget->sizeHint().height();
    topPanelWidget->setFixedHeight(height);
    setViewportMargins(leftMarginWidth(), height, 0, 0);
}

void TextEditor::updateLeftMarginArea(const QRect &rect, int dy)
{
    if (dy) {
        leftMargin->scroll(0, dy);
    } else {
        leftMargin->update(0, rect.y(), leftMargin->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLeftMarginAreaWidth();
    }
}

void TextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    leftMargin->setGeometry(QRect(cr.left(), cr.top() + topPanelWidget->sizeHint().height(), leftMarginWidth(), cr.height()));

    findReplaceWidget->setFixedHeight(findReplaceWidget->sizeHint().height());
    findReplaceWidget->move(this->width() - findReplaceWidget->width() - 9 - QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent), 9);

    topPanelWidget->setFixedWidth(this->width());
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    highlightedLine = textCursor().block().firstLineNumber();

    if (!this->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = this->palette().color(QPalette::Window).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.format.setProperty(QTextFormat::UserFormat, "currentHighlight");
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelectionGroup("lineHighlight", extraSelections);
}

void TextEditor::leftMarginPaintEvent(QPaintEvent *event)
{
    QPainter painter(leftMargin);
    painter.fillRect(event->rect(), this->palette().color(QPalette::Window).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        int lineNo = blockNumber + 1;

        for (MergeLines mergeBlock : mergedLines) {
            for (int i = 0; i < mergeBlock.length; i++) {
                int mergeLine = mergeBlock.startLine + i;
                if (blockNumber == mergeLine) {
                    painter.setPen(Qt::transparent);
                    if (mergeDecisions.value(mergeBlock)) {
                        painter.setBrush(QColor(0, 150, 0));
                    } else {
                        painter.setBrush(QColor(100, 100, 100));
                    }
                    painter.drawRect(0, top, leftMargin->width(), bottom - top);
                }
            }
        }

        QString number = QString::number(lineNo);
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(this->palette().color(QPalette::WindowText));
            painter.drawText(0, top, leftMargin->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        if (lineNo == brokenLine) {
            painter.setBrush(Qt::yellow);
            QPolygon polygon;
            polygon.append(QPoint(1, top));
            polygon.append(QPoint(1, bottom));
            polygon.append(QPoint(15, (top + bottom) / 2));
            painter.drawPolygon(polygon);
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
        if (brokenLine == lineNo) {
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

    leftMargin->repaint();
    highlightCurrentLine();
}

QList<QTextEdit::ExtraSelection> TextEditor::extraSelectionGroup(QString extraSelectionGroup) {
    return extraSelectionGroups.value(extraSelectionGroup);
}

void TextEditor::setExtraSelectionGroup(QString extraSelectionGroup, QList<QTextEdit::ExtraSelection> selections) {
    extraSelectionGroups.insert(extraSelectionGroup, selections);
    updateExtraSelections();
}

void TextEditor::clearExtraSelectionGroup(QString extraSelectionGroups) {
    this->extraSelectionGroups.remove(extraSelectionGroups);
    updateExtraSelections();
}

void TextEditor::updateExtraSelections() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    for (QList<QTextEdit::ExtraSelection> extraSelectionGroup : extraSelectionGroups.values()) {
        extraSelections.append(extraSelectionGroup);
    }

    setExtraSelections(extraSelections);
}

void TextEditor::setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections) {
    leftMargin->repaint();
    QPlainTextEdit::setExtraSelections(extraSelections);
}

void TextEditor::openFileFake(QString filename, QString contents) {
    this->setPlainText(contents);

    edited = false;
    this->fn = filename;
    emit fileNameChanged();
    emit editedChanged();
}

void TextEditor::toggleFindReplace() {
    if (findReplaceWidget->isVisible()) {
        findReplaceWidget->setVisible(false);
    } else {
        findReplaceWidget->setVisible(true);
        findReplaceWidget->setFocus();
    }
}

void TextEditor::revertFile() {
    if (this->fn != "") {
        QMessageBox* messageBox = new QMessageBox(this->window());
        messageBox->setWindowTitle(tr("Revert Changes?"));
        messageBox->setText(tr("Do you want to revert all the edits made to this document?"));
        messageBox->setIcon(QMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox->setDefaultButton(QMessageBox::Save);
        int button = messageBox->exec();

        if (button == QMessageBox::Yes) {
            openFile(this->fn);
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
        cursorBeforeDrop = this->textCursor();
        this->setTextCursor(this->cursorForPosition(event->pos()));
        return;
    }

    event->setDropAction(Qt::IgnoreAction);
}

void TextEditor::dragLeaveEvent(QDragLeaveEvent *event) {
    this->setTextCursor(cursorBeforeDrop);
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
                parentWindow->newTab(url.toLocalFile());
            }
        }
    } else if (data->hasText()) {
        this->cursorForPosition(event->pos()).insertText(data->text());
    }
}

bool TextEditor::saveFileAskForFilename(bool saveAs) {
    if (this->filename() == "" || saveAs) {
        QEventLoop* loop = new QEventLoop();
        QFileDialog* saveDialog = new QFileDialog(this->window(), Qt::Sheet);
        saveDialog->setWindowModality(Qt::WindowModal);
        saveDialog->setAcceptMode(QFileDialog::AcceptSave);
        saveDialog->setDirectory(QDir::home());
        saveDialog->setNameFilters(QStringList() << "Text File (*.txt)"
                                                 << "All Files (*)");
        connect(saveDialog, SIGNAL(finished(int)), saveDialog, SLOT(deleteLater()));
        connect(saveDialog, SIGNAL(finished(int)), loop, SLOT(quit()));
        saveDialog->show();

        //Block until dialog is finished
        loop->exec();
        loop->deleteLater();

        if (saveDialog->result() == QDialog::Accepted) {
            return this->saveFile(saveDialog->selectedFiles().first());
        } else {
            return false;
        }
    } else {
        return this->saveFile();
    }
}

void TextEditor::addTopPanel(QWidget* topPanel) {
    topPanelLayout->addWidget(topPanel);
    topPanel->setVisible(true);
    updateLeftMarginAreaWidth();

    if (qobject_cast<TopNotification*>(topPanel)) {
        emit primaryTopNotificationChanged(qobject_cast<TopNotification*>(topPanel));
    }
}

void TextEditor::removeTopPanel(QWidget* topPanel) {
    topPanelLayout->removeWidget(topPanel);
    topPanel->setVisible(false);
    updateLeftMarginAreaWidth();

    emit primaryTopNotificationChanged(nullptr);
}

void TextEditor::fileOnDiskChanged() {
    addTopPanel(onDiskChanged);
}

void TextEditor::lockScrolling(TextEditor *other) {
    if (this->scrollingLock != other) {
        this->scrollingLock = other;
        other->lockScrolling(this);
    }
}

void TextEditor::setMergedLines(QList<MergeLines> mergedLines) {
    this->mergedLines = mergedLines;
    for (MergeLines mergeBlock : mergedLines) {
        mergeDecisions.insert(mergeBlock, false);
    }
    updateMergedLinesColour();
}

void TextEditor::toggleMergedLines(int line) {
    for (MergeLines mergeBlock : mergedLines) {
        for (int i = 0; i < mergeBlock.length; i++) {
            int mergeLine = mergeBlock.startLine + i;
            if (mergeLine == line - 1) {
                bool currentDecision = mergeDecisions.value(mergeBlock);
                mergeDecisions.insert(mergeBlock, !currentDecision);
                updateMergedLinesColour();
                emit mergeDecision(mergeBlock, !currentDecision);
                leftMargin->update();
                return;
            }
        }
    }
}

void TextEditor::updateMergedLinesColour() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    for (MergeLines mergeBlock : mergedLines) {
        QTextCursor cur(this->document());
        QBrush lineColor;
        if (mergeDecisions.value(mergeBlock)) {
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
    return mergeDecisions.value(mergedLine);
}

void TextEditor::setHighlighter(QSyntaxHighlighter *hl) {
    if (this->hl != nullptr) {
        this->hl->deleteLater();
    }
    this->hl = hl;
    if (hl != nullptr) {
        hl->setDocument(this->document());
        hl->rehighlight();
    }
}

void TextEditor::reloadSettings() {
    QFont f;
    if (settings.value("font/useSystem", true).toBool()) {
        f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    } else {
        f = QFont(settings.value("font/textFontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString(), settings.value("font/textFontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());
    }
    this->setFont(f);
}
