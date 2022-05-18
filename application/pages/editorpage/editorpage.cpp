#include "editorpage.h"
#include "ui_editorpage.h"

#include <editormanager.h>
#include <editors/abstracteditor/abstracteditor.h>
#include <statemanager.h>
#include <twindowtabberbutton.h>

struct EditorPagePrivate {
        tWindowTabberButton* tabButton;
        AbstractEditor* editor = nullptr;
};

EditorPage::EditorPage(QString editorType, QWidget* parent) :
    AbstractPage(parent),
    ui(new Ui::EditorPage) {
    ui->setupUi(this);

    d = new EditorPagePrivate;
    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText(tr("Empty Document"));

    d->editor = StateManager::editor()->createEditor(editorType);
    if (d->editor) {
        ui->stackedWidget->addWidget(d->editor);
        ui->stackedWidget->setCurrentWidget(d->editor, false);
    }
}

EditorPage::~EditorPage() {
    delete ui;
    delete d;
}

void EditorPage::undo() {
    if (d->editor) d->editor->undo();
}

void EditorPage::redo() {
    if (d->editor) d->editor->redo();
}

tWindowTabberButton* EditorPage::tabButton() {
    return d->tabButton;
}
