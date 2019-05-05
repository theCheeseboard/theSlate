#include "textstatusbar.h"
#include "ui_textstatusbar.h"

#include <QMenu>
#include <QActionGroup>
#include <Repository>
#include <QLineEdit>
#include <QWidgetAction>
#include "texteditor.h"

extern KSyntaxHighlighting::Repository* highlightRepo;

struct TextStatusBarPrivate {
    TextEditor* editor;
    int lineEndings = -1;

    QActionGroup* lineEndingsGroup;
    QActionGroup* syntaxHighlightingGroup;
    QMap<QString, QAction*> syntaxHighlightingActions;

    QSettings settings;
};

TextStatusBar::TextStatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextStatusBar)
{
    ui->setupUi(this);

    d = new TextStatusBarPrivate();
}

TextStatusBar::~TextStatusBar()
{
    delete d;
    delete ui;
}

void TextStatusBar::setEditor(TextEditor* editor) {
    d->editor = editor;

    d->lineEndingsGroup = new QActionGroup(this);
    QMenu* lineEndingsMenu = new QMenu();
    lineEndingsMenu->addSection(tr("Line Endings for this file"));
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

    //Load Syntax Highlighters
    d->syntaxHighlightingGroup = new QActionGroup(this);
    QMenu* syntaxHighlightingMenu = new QMenu();
    QAction* noneSyntaxHighlighting = syntaxHighlightingMenu->addAction(tr("No Highlighting"), [=] {
        this->d->editor->setHighlighter(highlightRepo->definitionForName("None"));
    });
    d->syntaxHighlightingGroup->addAction(noneSyntaxHighlighting);
    d->syntaxHighlightingActions.insert("None", noneSyntaxHighlighting);
    noneSyntaxHighlighting->setCheckable(true);

    QMenu* sectionMenu = nullptr;
    for (KSyntaxHighlighting::Definition d : highlightRepo->definitions()) {
        if (sectionMenu == nullptr || sectionMenu->property("sectionName") != d.section()) {
            sectionMenu = new QMenu();
            sectionMenu->setTitle(" " + d.translatedSection());
            sectionMenu->setProperty("sectionName", d.section());

            if (d.section() != "") {
                syntaxHighlightingMenu->addMenu(sectionMenu);
            }
        }

        QAction* action = sectionMenu->addAction(d.translatedName(), [=] {
            this->d->editor->setHighlighter(d);
        });
        this->d->syntaxHighlightingGroup->addAction(action);
        this->d->syntaxHighlightingActions.insert(d.name(), action);
        action->setCheckable(true);
    }

    //ui->highlightingButton->setMenu(syntaxHighlightingMenu);

    setLineEndings(-1);
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

void TextStatusBar::setHighlighting(KSyntaxHighlighting::Definition definition) {
    ui->highlightingButton->setText(definition.translatedName());

    if (d->syntaxHighlightingActions.contains(definition.name())) {
        d->syntaxHighlightingActions.value(definition.name())->setChecked(true);
    }
}

void TextStatusBar::on_highlightingButton_clicked()
{
    d->editor->chooseHighlighter();
}

void TextStatusBar::setEncoding(QString encodingName) {
    ui->encodingButton->setText(encodingName);
}

void TextStatusBar::on_encodingButton_clicked()
{
    d->editor->chooseCodec();
}

void TextStatusBar::on_positionButton_clicked()
{
    d->editor->gotoLine();
}
