#include "texteditor.h"

#include <QStyle>
#include <QMessageBox>
#include <QMimeData>
#include "the-libs_global.h"
#include "mainwindow.h"

TextEditor::TextEditor(MainWindow *parent) : QPlainTextEdit(parent)
{
    this->setLineWrapMode(NoWrap);
    QFont normalFont = this->font();
    this->parentWindow = parent;

    button = new TabButton(this);
    button->setText(tr("New Document"));
    this->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    hl = new SyntaxHighlighter(this->document());

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
    });

    leftMargin = new TextEditorLeftMargin(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLeftMarginAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLeftMarginArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    leftMargin->setVisible(true);
    updateLeftMarginAreaWidth(0);
    highlightCurrentLine();

    findReplaceWidget = new FindReplace(this);
    findReplaceWidget->setFont(normalFont);
    findReplaceWidget->setFixedWidth(500 * theLibsGlobal::getDPIScaling());
    findReplaceWidget->setFixedHeight(findReplaceWidget->sizeHint().height());
    findReplaceWidget->hide();
    //findReplaceWidget->show();
}

TextEditor::~TextEditor() {
    if (button != NULL) {
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
    QFile f(file);
    f.open(QFile::ReadOnly | QFile::Text);
    this->setPlainText(f.readAll());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();

    QFileInfo fileInfo(file);
    if (fileInfo.suffix() == "cpp") { //C++ File
        hl->setCodeType(SyntaxHighlighter::cpp);
    } else if (fileInfo.suffix() == "py") { //Python File
        hl->setCodeType(SyntaxHighlighter::py);
    } else if (fileInfo.suffix() == "js") { //Javascript File
        hl->setCodeType(SyntaxHighlighter::js);
    } else if (fileInfo.suffix() == "json") { //JSON File
        hl->setCodeType(SyntaxHighlighter::json);
    } else if (fileInfo.suffix() == "tslprj") { //theSlate Project File
        hl->setCodeType(SyntaxHighlighter::json);
    }

    if (git != nullptr) {
        git->deleteLater();
    }
    git = new GitIntegration(fileInfo.absoluteDir());
}

bool TextEditor::saveFile(QString file) {
    QFile f(file);
    f.open(QFile::WriteOnly | QFile::Text);
    f.write(this->toPlainText().toUtf8());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();

    git = new GitIntegration(QFileInfo(file).absoluteDir());

    return true;
}

bool TextEditor::saveFile() {
    if (this->filename() == "") {
        return false;
    } else {
        return saveFile(this->filename());
    }
}

SyntaxHighlighter* TextEditor::highlighter() {
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
    } else if (event->text() == "\"" && highlighter()->currentCodeType() != SyntaxHighlighter::none) {
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

    return space;
}

void TextEditor::updateLeftMarginAreaWidth(int newBlockCount)
{
    setViewportMargins(leftMarginWidth(), 0, 0, 0);
}

void TextEditor::updateLeftMarginArea(const QRect &rect, int dy)
{
    if (dy) {
        leftMargin->scroll(0, dy);
    } else {
        leftMargin->update(0, rect.y(), leftMargin->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLeftMarginAreaWidth(0);
    }
}

void TextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    leftMargin->setGeometry(QRect(cr.left(), cr.top(), leftMarginWidth(), cr.height()));

    findReplaceWidget->setFixedHeight(findReplaceWidget->sizeHint().height());
    findReplaceWidget->move(this->width() - findReplaceWidget->width() - 9 - QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent), 9);
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
