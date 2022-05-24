#include "projectpage.h"
#include "ui_projectpage.h"

#include <editormanager.h>
#include <project.h>
#include <statemanager.h>
#include <twindowtabberbutton.h>

#include "../editorpage/editorpage.h"
#include "leftpanes/filetreeleftpane/filetreeleftpane.h"

struct ProjectPagePrivate {
        tWindowTabberButton* tabButton;
        ProjectPtr project;

        QMap<QUrl, EditorPage*> editors;
};

ProjectPage::ProjectPage(QString projectDirectory, QWidget* parent) :
    AbstractPage(parent),
    ui(new Ui::ProjectPage) {
    ui->setupUi(this);

    d = new ProjectPagePrivate();
    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText("Project");

    d->project = ProjectPtr(new Project(projectDirectory));

    ui->leftPaneStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
    ui->leftPaneContainer->setFixedWidth(SC_DPI_W(300, this));

    auto fileTreeLeftPane = new FileTreeLeftPane(d->project);
    connect(fileTreeLeftPane, &FileTreeLeftPane::requestFileOpen, this, &ProjectPage::openUrl);
    addLeftPaneItem(fileTreeLeftPane);
}

ProjectPage::~ProjectPage() {
    delete ui;
    delete d;
}

void ProjectPage::addLeftPaneItem(AbstractLeftPane* leftPane) {
    ui->leftPaneStack->addWidget(leftPane);
    ui->leftPaneTabber->addButton(leftPane->tabButton());
    leftPane->tabButton()->syncWithStackedWidget(ui->leftPaneStack, leftPane);
}

void ProjectPage::openUrl(QUrl url) {
    if (d->editors.contains(url)) {
        ui->stackedWidget->setCurrentWidget(d->editors.value(url));
    }

    QString editorType = StateManager::editor()->editorTypeForFileName(url.fileName());
    auto* editor = new EditorPage(editorType);
    editor->discardContentsAndOpenFile(url);
    ui->stackedWidget->addWidget(editor);
    ui->stackedWidget->setCurrentWidget(editor);
    d->editors.insert(url, editor);
}

tWindowTabberButton* ProjectPage::tabButton() {
    return d->tabButton;
}

void ProjectPage::undo() {
    auto editor = qobject_cast<EditorPage*>(ui->stackedWidget->currentWidget());
    if (editor) editor->undo();
}

void ProjectPage::redo() {
    auto editor = qobject_cast<EditorPage*>(ui->stackedWidget->currentWidget());
    if (editor) editor->undo();
}

tPromise<void>* ProjectPage::save() {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

tPromise<void>* ProjectPage::saveAs() {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

tPromise<void>* ProjectPage::saveAll() {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

tPromise<void>* ProjectPage::saveBeforeClose(bool silent) {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

bool ProjectPage::saveAndCloseShouldAskUserConfirmation() {
    return false;
}
