#include "projectpage.h"
#include "ui_projectpage.h"

#include <QAction>
#include <editormanager.h>
#include <project.h>
#include <statemanager.h>
#include <twindowtabberbutton.h>

#include "../editorpage/editorpage.h"
#include "leftpanes/filetreeleftpane/filetreeleftpane.h"
#include "leftpanes/runconfigurationleftpane/runconfigurationleftpane.h"

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

    d->project = Project::createProject(projectDirectory);
    d->project->addBeforeBuildEventHandler([=] {
        return this->saveAll();
    });

    ui->leftPaneStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
    ui->leftPaneContainer->setFixedWidth(SC_DPI_W(300, this));
    ui->buildTasks->setProject(d->project);

    auto* fileTreeLeftPane = new FileTreeLeftPane(d->project);
    connect(fileTreeLeftPane, &FileTreeLeftPane::requestFileOpen, this, &ProjectPage::openUrl);
    addLeftPaneItem(fileTreeLeftPane);

    auto* runConfigLeftPane = new RunConfigurationLeftPane(d->project);
    connect(runConfigLeftPane, &RunConfigurationLeftPane::requestFileOpen, this, &ProjectPage::openUrl);
    addLeftPaneItem(runConfigLeftPane);

    auto* buildAction = new QAction(this);
    buildAction->setText(tr("Build"));
    buildAction->setIcon(QIcon::fromTheme("package")); // TODO: Make a better icon
    connect(buildAction, &QAction::triggered, this, [=] {
        d->project->activeRunConfigurationBuild();
    });
    d->tabButton->addAction(buildAction);

    auto* runAction = new QAction();
    runAction->setText("run");
    runAction->setIcon(QIcon::fromTheme("media-playback-start"));
    connect(runAction, &QAction::triggered, this, [=] {
        d->project->activeRunConfigurationRun();
    });
    d->tabButton->addAction(runAction);

    runAction->setEnabled(d->project->canActiveRunConfigurationRun());
    connect(d->project.data(), &Project::currentTargetChanged, this, [=] {
        runAction->setEnabled(d->project->canActiveRunConfigurationRun());
    });
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
    this->saveAll();

    QUrl canonicalUrl = url.adjusted(QUrl::RemoveQuery);

    if (d->editors.contains(canonicalUrl)) {
        ui->stackedWidget->setCurrentWidget(d->editors.value(canonicalUrl));
    }

    QString editorType = StateManager::editor()->editorTypeForUrl(url);
    auto* editor = new EditorPage(editorType);
    editor->discardContentsAndOpenFile(url);
    ui->stackedWidget->addWidget(editor);
    ui->stackedWidget->setCurrentWidget(editor);
    d->editors.insert(canonicalUrl, editor);
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
    if (editor) editor->redo();
}

tPromise<void>* ProjectPage::save() {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

tPromise<void>* ProjectPage::saveAs() {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

tPromise<void>* ProjectPage::saveAll() {
    return TPROMISE_CREATE_NEW_THREAD(void, {
        for (int i = 0; i < ui->stackedWidget->count(); i++) {
            auto editor = qobject_cast<EditorPage*>(ui->stackedWidget->widget(i));
            if (editor) editor->saveAll()->await();
        }

        res();
    });
}

tPromise<void>* ProjectPage::saveBeforeClose(bool silent) {
    return TPROMISE_CREATE_SAME_THREAD(void, {});
}

bool ProjectPage::saveAndCloseShouldAskUserConfirmation() {
    return false;
}
