#include "texteditor.h"

TextEditor::TextEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    this->setLineWrapMode(NoWrap);

    button = new TabButton(this);
    button->setText("New Document");
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

    QStringList args = QApplication::instance()->arguments();
    if (args.count() > 1) {
        openFile(args[1]);
    }
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
    f.open(QFile::ReadOnly);
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
}

bool TextEditor::saveFile(QString file) {
    QFile f(file);
    f.open(QFile::WriteOnly);
    f.write(this->toPlainText().toUtf8());
    f.close();

    edited = false;
    this->fn = file;
    emit fileNameChanged();
    emit editedChanged();
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
    } else if (event->text() == "\"") {
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
    } else if (event->text() == "'") {
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

    if (currentException.errorType != "") {
        QTextBlock block = this->document()->findBlockByLineNumber(currentException.line);
        QRect bounding = blockBoundingGeometry(block).translated(contentOffset()).toRect();

        exceptionDialog->move(0, bounding.top());
    }
}

void TextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    if (ideMode) {
        QRect cr = contentsRect();
        leftMargin->setGeometry(QRect(cr.left(), cr.top(), leftMarginWidth(), cr.height()));
        if (currentException.errorType != "") {
            QTextBlock block = this->document()->findBlockByLineNumber(currentException.line);
            QRect bounding = blockBoundingGeometry(block).translated(contentOffset()).toRect();

            exceptionDialog->move(0, bounding.top());
        }
    }
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();
    if (highlightedLine != -1) {
        for (int i = 0; i < extraSelections.count(); i++) {
            if (extraSelections.at(i).format.property(QTextFormat::UserFormat) == "currentHighlight") {
                extraSelections.removeAt(i);
            }
        }
    }

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

    setExtraSelections(extraSelections);
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

        if (breakpoints.contains(block)) {
            painter.setBrush(Qt::red);
            painter.drawEllipse(1, top, fontMetrics().height() - 2, fontMetrics().height() - 2);
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

void TextEditor::addBreakpoint(int lineNumber) {
    QTextBlock block = this->document()->findBlockByLineNumber(lineNumber - 1);
    breakpoints.append(block);

    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(Qt::red);
    selection.format.setForeground(Qt::white);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.format.setProperty(QTextFormat::UserFormat, "breakpoint");
    selection.format.setProperty(QTextFormat::UserFormat + 1, lineNumber);
    selection.cursor = QTextCursor(block);
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
    emit breakpointSet(lineNumber);
    //reloadBlockHighlighting();
}

void TextEditor::removeBreakpoint(int lineNumber) {
    QTextBlock block = this->document()->findBlockByLineNumber(lineNumber - 1);
    breakpoints.removeAll(block);

    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    for (int i = 0; i < extraSelections.count(); i++) {
        if (extraSelections.at(i).format.property(QTextFormat::UserFormat + 1) == lineNumber) {
            extraSelections.removeAt(i);
        }
    }

    setExtraSelections(extraSelections);
    emit breakpointRemoved(lineNumber);
    //reloadBlockHighlighting();
}

void TextEditor::reloadBlockHighlighting() {
    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

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
        } else if (breakpoints.contains(block)) {
            QTextEdit::ExtraSelection selection;

            selection.format.setBackground(Qt::red);
            selection.format.setForeground(Qt::white);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setProperty(QTextFormat::UserFormat, "breakpoint");
            selection.cursor = cursorForPosition(QPoint(blockBoundingGeometry(block).top() + 1, 0));
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
    setExtraSelections(extraSelections);

    leftMargin->repaint();
    highlightCurrentLine();
}

bool TextEditor::hasBreakpoint(int lineNumber) {
    return breakpoints.contains(this->document()->findBlockByLineNumber(lineNumber - 1));
}

void TextEditor::setBrokenLine(int lineNumber) {
    QTextBlock block = this->document()->findBlockByLineNumber(lineNumber - 1);

    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(Qt::yellow);
    selection.format.setForeground(Qt::black);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.format.setProperty(QTextFormat::UserFormat, "breakingLine");
    selection.cursor = QTextCursor(block);
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
    emit breakpointSet(lineNumber);

    brokenLine = lineNumber;
    //reloadBlockHighlighting();
}

void TextEditor::clearBrokenLine() {
    QTextBlock block = this->document()->findBlockByLineNumber(brokenLine - 1);

    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    for (int i = 0; i < extraSelections.count(); i++) {
        if (extraSelections.at(i).format.property(QTextFormat::UserFormat) == "breakingLine") {
            extraSelections.removeAt(i);
        }
    }

    setExtraSelections(extraSelections);

    brokenLine = -1;
    //reloadBlockHighlighting();
}

void TextEditor::setExtraSelections(const QList<QTextEdit::ExtraSelection> &extraSelections) {
    leftMargin->repaint();
    QPlainTextEdit::setExtraSelections(extraSelections);
}

void TextEditor::exceptionPaintEvent(QPaintEvent *event) {
    if (currentException.errorType != "") {
        QPainter painter(exceptionDialog);
        painter.fillRect(event->rect(), QColor(255, 100, 100));

        QFont font = QApplication::font();
        QFontMetrics metrics = QApplication::fontMetrics();

        painter.setPen(Qt::white);

        QRect titleRect(9, 9, exceptionDialog->width() - 18, metrics.height());

        font.setBold(true);
        painter.setFont(font);
        painter.drawText(titleRect, "Exception: " + currentException.errorType);

        painter.setFont(this->font());
        QRect descRect(9, titleRect.bottom(), exceptionDialog->width() - 18, exceptionDialog->height() - 18 - metrics.height());
        painter.drawText(descRect, currentException.description);
    }
}

void TextEditor::setException(Exception exception) {
    currentException = exception;

    exceptionDialog->setVisible(true);
    QTextBlock block = this->document()->findBlockByLineNumber(currentException.line);
    QRect bounding = blockBoundingGeometry(block).translated(contentOffset()).toRect();

    exceptionDialog->move(0, bounding.top());
    exceptionDialog->resize(this->width() - 0, (exception.description.split("\n").count() + 1) * this->fontMetrics().height() + 18);
}

void TextEditor::clearException() {
    currentException = Exception();
    exceptionDialog->setVisible(false);
}

void TextEditor::enableIDEMode() {
    ideMode = true;

    leftMargin = new TextEditorLeftMargin(this);
    exceptionDialog = new ExceptionDialog(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLeftMarginAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLeftMarginArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    leftMargin->setVisible(true);
    updateLeftMarginAreaWidth(0);
    highlightCurrentLine();
}

QList<QTextBlock> TextEditor::allBreakpoints() {
    return breakpoints;
}

void TextEditor::openFileFake(QString filename, QString contents) {
    this->setPlainText(contents);

    edited = false;
    this->fn = filename;
    emit fileNameChanged();
    emit editedChanged();
}
