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
#ifndef LANDINGPAGE_H
#define LANDINGPAGE_H

#include <QWidget>

namespace Ui {
    class LandingPage;
}

class LandingPage : public QWidget {
        Q_OBJECT

    public:
        explicit LandingPage(QWidget* parent = nullptr);
        ~LandingPage();

    signals:
        void createNewFile();
        void openDirectory();
        void cloneRepository();

    private slots:
        void on_newFileButton_clicked();

        void on_openProjectButton_clicked();

        void on_cloneRepositoryButton_clicked();

    private:
        Ui::LandingPage* ui;
};

#endif // LANDINGPAGE_H
