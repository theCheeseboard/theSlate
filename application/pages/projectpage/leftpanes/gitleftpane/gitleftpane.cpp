/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "gitleftpane.h"
#include "ui_gitleftpane.h"

#include "gitroot.h"
#include <QFileSystemWatcher>
#include <twindowtabberbutton.h>

#include <objects/repository.h>

struct GitLeftPanePrivate {
        tWindowTabberButton* tabButton;
        ProjectPtr project;

        QFileSystemWatcher* watcher;

        GitRoot* gr = nullptr;
        RepositoryPtr repo;
};

GitLeftPane::GitLeftPane(ProjectPtr project, QWidget* parent) :
    AbstractLeftPane(parent),
    ui(new Ui::GitLeftPane) {
    ui->setupUi(this);
    d = new GitLeftPanePrivate();
    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText(tr("Git"));

    d->project = project;

    d->watcher = new QFileSystemWatcher();
    d->watcher->addPath(project->projectDir().absolutePath());
    connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &GitLeftPane::reloadGitState);
    this->reloadGitState();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
}

GitLeftPane::~GitLeftPane() {
    delete d;
    delete ui;
}

void GitLeftPane::reloadGitState() {
    RepositoryPtr repo;
    if (QDir(d->project->projectDir().absoluteFilePath(".git")).exists()) {
        repo = Repository::repositoryForDirectory(d->project->projectDir().absolutePath());
    }

    if (repo) {
        if (d->repo) return;

        d->repo = repo;
        d->gr = new GitRoot(repo, this);
        ui->gitPageLayout->addWidget(d->gr);
        ui->stackedWidget->setCurrentWidget(ui->gitPage);
    } else {
        d->repo.clear();
        ui->stackedWidget->setCurrentWidget(ui->noRepositoryPage);

        ui->gitPageLayout->removeWidget(d->gr);
        d->gr->deleteLater();
        d->gr = nullptr;
    }
}

tWindowTabberButton* GitLeftPane::tabButton() {
    return d->tabButton;
}
