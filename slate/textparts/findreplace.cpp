#include "findreplace.h"
#include "ui_findreplace.h"

#include <ttoast.h>
#include <terrorflash.h>

struct FindReplacePrivate {
    TextEditor* editor;

    QMap<int, int> matches;
    bool haveValidRegex = false;

    QAction* matchCaseAction;
    QAction* regexAction;
    QAction* wholeWordAction;
};

FindReplace::FindReplace(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindReplace)
{
    ui->setupUi(this);
    d = new FindReplacePrivate();

    this->setParent(parent);
    ui->matchesFound->setVisible(false);
    ui->regexErrorLabel->setVisible(false);

    ui->findBox->installEventFilter(this);

    QMenu* moreOptionsMenu = new QMenu();
    d->matchCaseAction = moreOptionsMenu->addAction(tr("Match case"));
    d->regexAction = moreOptionsMenu->addAction(tr("Regular expression"));
    d->wholeWordAction = moreOptionsMenu->addAction(tr("Whole word"));
    ui->moreOptionsButton->setMenu(moreOptionsMenu);

    d->matchCaseAction->setCheckable(true);
    d->regexAction->setCheckable(true);
    d->wholeWordAction->setCheckable(true);

    connect(moreOptionsMenu, &QMenu::triggered, this, [=] {
        on_findBox_textChanged(ui->findBox->text());
    });
}

FindReplace::~FindReplace()
{
    delete d;
    delete ui;
}

void FindReplace::setEditor(TextEditor *editor) {
    d->editor = editor;
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

QRegularExpression FindReplace::searchExpression(QString text) {
    QString pattern;
    if (d->regexAction->isChecked()) {
        pattern = text;
    } else {
        pattern = QRegularExpression::escape(text);
        if (d->wholeWordAction->isChecked()) {
            pattern.prepend("\\b");
            pattern.append("\\b");
        }
    }

    QRegularExpression searchTerm(pattern);
    if (!d->matchCaseAction->isChecked()) searchTerm.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    return searchTerm;
}

void FindReplace::find(QString text) {
    QTextCursor currentCursor = d->editor->textCursor();

    QRegularExpression searchTerm = searchExpression(text);

    if (searchTerm.isValid()) {
        d->haveValidRegex = true;
        ui->regexErrorLabel->setVisible(false);

        QList<QTextEdit::ExtraSelection> selections;
        QString document = d->editor->toPlainText();

        d->matches.clear();
        QRegularExpressionMatchIterator iterator = searchTerm.globalMatch(document);
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            if (match.hasMatch()) {
                d->matches.insert(match.capturedStart(), match.capturedEnd());
            }
        }

        for (int match : d->matches.keys()) {
            QTextCursor cur(d->editor->document());
            cur.setPosition(match);
            cur.setPosition(d->matches.value(match), QTextCursor::KeepAnchor);

            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor(255, 255, 0));
            selection.format.setForeground(QColor(0, 0, 0));
            selection.cursor = cur;
            selections.append(selection);
        }
        d->editor->setExtraSelectionGroup("findreplace", selections);

        int leftPosition = qMin(d->editor->textCursor().position(), d->editor->textCursor().anchor());
        for (int match : d->matches) {
            if (match >= leftPosition) {
                currentCursor.setPosition(match);
                currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, d->matches.value(match));
                break;
            }
        }
        d->editor->setTextCursor(currentCursor);

        if (d->matches.count() == 0) {
            ui->matchesFound->setText(tr("No matches"));
        } else {
            ui->matchesFound->setText(tr("%n matches", nullptr, selections.count()));
        }
    } else {
        d->haveValidRegex = false;
        ui->regexErrorLabel->setVisible(true);
        ui->matchesFound->setText(tr("Invalid Regular Expression"));
        ui->regexErrorLabel->setText(searchTerm.errorString());
    }
    ui->matchesFound->setVisible(true);
    this->repaint();
}

void FindReplace::on_doneButton_clicked()
{
    this->hide();
    d->editor->repaint();
}

void FindReplace::moveCursor(bool backward) {
    if (!d->haveValidRegex) {
        tErrorFlash::flashError(ui->findContainer);
    } else {
        QTextCursor currentCursor = d->editor->textCursor();

        int leftPosition = qMin(d->editor->textCursor().position(), d->editor->textCursor().anchor());
        if (backward) {
            int previousIndex = -1;
            for (int index : d->matches.keys()) {
                if (index < leftPosition) {
                    previousIndex = index;
                } else {
                    break;
                }
            }

            if (previousIndex != -1) {
                currentCursor.setPosition(previousIndex);
                currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, d->matches.value(previousIndex) - previousIndex);
            }
        } else {
            for (int index : d->matches.keys()) {
                if (index > leftPosition) {
                    currentCursor.setPosition(index);
                    currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, d->matches.value(index) - index);
                    break;
                }
            }
        }
        d->editor->setTextCursor(currentCursor);
    }
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
    ui->regexErrorLabel->setVisible(false);
    d->haveValidRegex = false;
    d->editor->clearExtraSelectionGroup("findreplace");
    d->matches.clear();
}

void FindReplace::setFocus() {
    ui->findBox->setFocus();
}

bool FindReplace::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->findBox) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* e = (QKeyEvent*) event;
            if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
                //Perform search
                if (e->modifiers() & Qt::ShiftModifier) {
                    moveCursor(true);
                } else {
                    moveCursor(false);
                }
                return true;
            } else if (e->key() == Qt::Key_Escape) {
                this->hide();
            }
        }
    }
    return false;
}

void FindReplace::on_replaceAllButton_clicked()
{
    QRegularExpression searchTerm = searchExpression(ui->findBox->text());

    if (searchTerm.isValid() && ui->findBox->text() != "") {
        QTextCursor currentCursor = d->editor->textCursor();

        QString originalDocument = d->editor->toPlainText();
        QString document = d->editor->toPlainText();
        document.replace(searchTerm, ui->replaceBox->text());
        d->editor->setPlainText(document);

        d->editor->setTextCursor(currentCursor);

        tToast* toast = new tToast();
        toast->setTitle(tr("Replaced all matches"));
        toast->setText(tr("%n occurences replaced", nullptr, d->matches.count()));
        toast->setActions({{"undo", tr("Undo")}});
        connect(toast, &tToast::actionClicked, this, [=](QString action) {
            if (action == "undo") {
                d->editor->setPlainText(originalDocument);
                d->editor->setTextCursor(currentCursor);
                find(ui->findBox->text());
            }
        });
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        toast->show(this->window());

        find(ui->findBox->text());
    } else {
        tErrorFlash::flashError(ui->findContainer);
    }
}
