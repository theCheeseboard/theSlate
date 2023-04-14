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
#ifndef GITLEFTPANE_H
#define GITLEFTPANE_H

#include "../abstractleftpane/abstractleftpane.h"
#include <QCoroTask>
#include <QWidget>
#include <project.h>

namespace Ui {
    class GitLeftPane;
}

class ProjectPage;
struct GitLeftPanePrivate;
class GitLeftPane : public AbstractLeftPane {
        Q_OBJECT

    public:
        explicit GitLeftPane(ProjectPtr project, ProjectPage* projectPage, QWidget* parent = nullptr);
        ~GitLeftPane();

    signals:
        void requestFileOpen(QUrl url);

    private:
        Ui::GitLeftPane* ui;
        GitLeftPanePrivate* d;

        void reloadGitState();

        // AbstractLeftPane interface
    public:
        tWindowTabberButton* tabButton();
    private slots:
        QCoro::Task<> on_initButton_clicked();
};

#endif // GITLEFTPANE_H
