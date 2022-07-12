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
    runAction->setText(tr("Run"));
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

QCoro::Task<> ProjectPage::openUrl(QUrl url) {
    co_await this->saveAll();

    QUrl canonicalUrl = url.adjusted(QUrl::RemoveQuery);

    if (d->editors.contains(canonicalUrl)) {
        ui->stackedWidget->setCurrentWidget(d->editors.value(canonicalUrl));
        co_return;
    }

    QString editorType = StateManager::editor()->editorTypeForUrl(url);
    auto* editor = new EditorPage(editorType);
    editor->setProject(d->project);
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

QCoro::Task<> ProjectPage::save() {
    co_return;
}

QCoro::Task<> ProjectPage::saveAs() {
    co_return;
}

QCoro::Task<> ProjectPage::saveAll() {
    for (int i = 0; i < ui->stackedWidget->count(); i++) {
        auto editor = qobject_cast<EditorPage*>(ui->stackedWidget->widget(i));
        if (editor) co_await editor->saveAll();
    }
}

QCoro::Task<> ProjectPage::saveBeforeClose(bool silent) {
    co_return;
}

bool ProjectPage::saveAndCloseShouldAskUserConfirmation() {
    return false;
}
