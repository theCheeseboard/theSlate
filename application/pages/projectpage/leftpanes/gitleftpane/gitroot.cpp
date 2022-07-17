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
#include "gitroot.h"
#include "ui_gitroot.h"

#include <popovers/snapinpopover.h>
#include <popovers/snapins/commitsnapin.h>
#include <popovers/snapins/pullsnapin.h>
#include <popovers/snapins/pushsnapin.h>

struct GitRootPrivate {
        RepositoryPtr repo;
};

GitRoot::GitRoot(RepositoryPtr repo, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GitRoot) {
    ui->setupUi(this);
    d = new GitRootPrivate();
    d->repo = repo;

    ui->commitButton->setIcon(QIcon::fromTheme("commit"));
    ui->pushButton->setIcon(QIcon::fromTheme("vcs-push"));
    ui->pullButton->setIcon(QIcon::fromTheme("vcs-pull"));

    ui->repositoryStatus->setRepository(repo);
}

GitRoot::~GitRoot() {
    delete d;
    delete ui;
}

void GitRoot::on_commitButton_clicked() {
    SnapInPopover::showSnapInPopover(this, new CommitSnapIn(d->repo));
}

void GitRoot::on_pushButton_clicked() {
    SnapInPopover::showSnapInPopover(this, new PushSnapIn(d->repo));
}

void GitRoot::on_pullButton_clicked() {
    SnapInPopover::showSnapInPopover(this, new PullSnapIn(d->repo));
}
