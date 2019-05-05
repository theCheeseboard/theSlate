#include "textwidget.h"
#include "ui_textwidget.h"

struct TextWidgetPrivate {
    MainWindow* parent;
};

TextWidget::TextWidget(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::TextWidget)
{
    ui->setupUi(this);
    d = new TextWidgetPrivate();
    d->parent = parent;

    ui->editor->setMainWindow(parent);
    ui->editor->setStatusBar(ui->statusBar);
    ui->findReplaceWidget->setEditor(ui->editor);
    ui->statusBar->setEditor(ui->editor);

    ui->findReplaceWidget->hide();
}

TextWidget::~TextWidget()
{
    delete d;
    delete ui;
}

TextEditor* TextWidget::editor() {
    return ui->editor;
}

void TextWidget::showFindReplace() {
    ui->findReplaceWidget->show();
    ui->findReplaceWidget->setFocus();
}
