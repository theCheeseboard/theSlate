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
#include "landingpage.h"
#include "ui_landingpage.h"

#include <tapplication.h>
#include <tcontentsizer.h>

LandingPage::LandingPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LandingPage) {
    ui->setupUi(this);

    ui->appNameLabel->setText(tApplication::applicationDisplayName());
    ui->appIconLabel->setPixmap(tApplication::applicationIcon().pixmap(QSize(ui->appNameLabel->sizeHint().height(), ui->appNameLabel->sizeHint().height())));
    new tContentSizer(ui->containerWidget);

    ui->newFileButton->setIcon(QIcon::fromTheme("document-new"));
    ui->openProjectButton->setIcon(QIcon::fromTheme("document-open"));
    ui->cloneRepositoryButton->setIcon(QIcon::fromTheme("edit-copy"));
}

LandingPage::~LandingPage() {
    delete ui;
}

void LandingPage::on_newFileButton_clicked() {
    emit createNewFile();
}

void LandingPage::on_openProjectButton_clicked() {
    emit openDirectory();
}

void LandingPage::on_cloneRepositoryButton_clicked() {
    emit cloneRepository();
}
