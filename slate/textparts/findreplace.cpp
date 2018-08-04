#include "findreplace.h"
#include "ui_findreplace.h"

FindReplace::FindReplace(TextEditor *parent) :
    QWidget(parent),
    ui(new Ui::FindReplace)
{
    ui->setupUi(this);

    editor = parent;
    this->setParent(parent);
    ui->matchesFound->setVisible(false);
}

FindReplace::~FindReplace()
{
    delete ui;
}

void FindReplace::on_findBox_textChanged(const QString &arg1)
{
    if (arg1 == "") {
        reset();
    } else {
        find(ui->findBox->text());
    }
}

void FindReplace::on_findNext_clicked()
{
    moveCursor();
}

void FindReplace::on_findPrev_clicked()
{
    moveCursor(true);
}

void FindReplace::find(QString text) {
    QTextCursor currentCursor = editor->textCursor();
    indices.clear();
    textLength = text.length();

    QList<QTextEdit::ExtraSelection> selections;
    QString document = editor->toPlainText();
    int startIndex = document.indexOf(text);
    while (startIndex != -1) {
        indices.append(startIndex);

        QTextCursor cur(editor->document());
        cur.setPosition(startIndex);
        cur.setPosition(startIndex + text.length(), QTextCursor::KeepAnchor);

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(255, 255, 0));
        selection.cursor = cur;
        selections.append(selection);
        startIndex = document.indexOf(text, startIndex + 1);
    }
    editor->setExtraSelectionGroup("findreplace", selections);

    int leftPosition = qMin(editor->textCursor().position(), editor->textCursor().anchor());
    for (int index : indices) {
        if (index >= leftPosition) {
            currentCursor.setPosition(index);
            currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.length());
            break;
        }
    }
    editor->setTextCursor(currentCursor);

    if (selections.count() == 0) {
        ui->matchesFound->setText(tr("No matches"));
    } else {
        ui->matchesFound->setText(tr("%n matches", nullptr, selections.count()));
    }
    ui->matchesFound->setVisible(true);
    this->repaint();
}

void FindReplace::on_doneButton_clicked()
{
    this->hide();
    editor->repaint();
}

void FindReplace::moveCursor(bool backward) {
    QTextCursor currentCursor = editor->textCursor();

    int leftPosition = qMin(editor->textCursor().position(), editor->textCursor().anchor());
    if (backward) {
        int previousIndex = -1;
        for (int index : indices) {
            if (index < leftPosition) {
                previousIndex = index;
            } else {
                break;
            }
        }

        if (previousIndex != -1) {
            currentCursor.setPosition(previousIndex);
            currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, textLength);
        }
    } else {
        for (int index : indices) {
            if (index > leftPosition) {
                currentCursor.setPosition(index);
                currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, textLength);
                break;
            }
        }
    }
    editor->setTextCursor(currentCursor);
}

void FindReplace::hideEvent(QHideEvent *event) {
    Q_UNUSED(event)
    reset();
}

void FindReplace::showEvent(QShowEvent *event) {
    Q_UNUSED(event)
    on_findBox_textChanged(ui->findBox->text());
}

void FindReplace::reset() {
    ui->matchesFound->setVisible(false);
    editor->clearExtraSelectionGroup("findreplace");
    indices.clear();
}
