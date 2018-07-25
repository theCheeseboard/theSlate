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
        ui->matchesFound->setVisible(false);
        editor->clearExtraSelectionGroup("findreplace");
    } else {
        find(ui->findBox->text());
    }
}

void FindReplace::on_findNext_clicked()
{
    find(ui->findBox->text());
}

void FindReplace::on_findPrev_clicked()
{
    find(ui->findBox->text(), true);
}

void FindReplace::find(QString text, bool backward) {
    QTextCursor currentCursor = editor->textCursor();

    QList<QTextEdit::ExtraSelection> selections;
    editor->moveCursor(QTextCursor::Start);
    while (editor->find(text, backward ? QTextDocument::FindBackward : QTextDocument::FindFlags())) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(255, 255, 0));
        selection.cursor = editor->textCursor();
        selections.append(selection);
    }

    editor->setExtraSelectionGroup("findreplace", selections);
    editor->setTextCursor(currentCursor);

    if (selections.count() == 0) {
        ui->matchesFound->setText(tr("No matches"));
    } else {
        ui->matchesFound->setText(tr("%n matches", nullptr, selections.count()));
    }
    ui->matchesFound->setVisible(true);
}
