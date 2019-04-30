#include "textstatusbar.h"
#include "ui_textstatusbar.h"

#include <QMenu>
#include <QActionGroup>
#include "texteditor.h"

struct TextStatusBarPrivate {
    TextEditor* editor;
    int lineEndings = -1;

    QActionGroup* lineEndingsGroup;

    QSettings settings;
};

TextStatusBar::TextStatusBar(TextEditor *parent) :
    QWidget(parent),
    ui(new Ui::TextStatusBar)
{
    ui->setupUi(this);

    d = new TextStatusBarPrivate();
    d->editor = parent;

    d->lineEndingsGroup = new QActionGroup(this);
    QMenu* lineEndingsMenu = new QMenu();
    d->lineEndingsGroup->addAction(lineEndingsMenu->addAction("LF", [=] {
        setLineEndings(0);
    }));
    d->lineEndingsGroup->addAction(lineEndingsMenu->addAction("CR", [=] {
        setLineEndings(1);
    }));
    d->lineEndingsGroup->addAction(lineEndingsMenu->addAction("CR LF", [=] {
        setLineEndings(2);
    }));
    ui->lineEndings->setMenu(lineEndingsMenu);
    for (QAction* action : d->lineEndingsGroup->actions()) {
        action->setCheckable(true);
    }

    setLineEndings(-1);
}

TextStatusBar::~TextStatusBar()
{
    delete d;
    delete ui;
}

void TextStatusBar::setPosition(int line, int col) {
    ui->positionButton->setText(tr("Ln %1, Col %2").arg(line + 1).arg(col + 1));
}

void TextStatusBar::setSpacing(bool spaces, int number) {
    if (spaces) {
        ui->tabSpacingButton->setText(tr("%n spaces", nullptr, number));
    } else {
        ui->tabSpacingButton->setText(tr("%n tabs", nullptr, number));
    }
}

void TextStatusBar::setLineEndings(int lineEndings) {
    if (lineEndings < 0) {
        setLineEndings(d->settings.value("behaviour/endOfLine", THESLATE_END_OF_LINE).toInt());
    } else {
        switch (lineEndings) {
            case 0: //UNIX
                ui->lineEndings->setText("LF");
                d->lineEndingsGroup->actions().at(0)->setChecked(true);
                break;
            case 1: //Macintosh
                ui->lineEndings->setText("CR");
                d->lineEndingsGroup->actions().at(1)->setChecked(true);
                break;
            case 2: //Windows
                ui->lineEndings->setText("CR LF");
                d->lineEndingsGroup->actions().at(2)->setChecked(true);
                break;
        }

        d->lineEndings = lineEndings;
    }
}

int TextStatusBar::lineEndings() {
    return d->lineEndings;
}
