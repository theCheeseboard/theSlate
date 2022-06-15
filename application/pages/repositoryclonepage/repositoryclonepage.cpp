#include "repositoryclonepage.h"
#include "ui_repositoryclonepage.h"

#include <objects/repository.h>
#include <twindowtabberbutton.h>

#include "../projectpage/projectpage.h"

struct RepositoryClonePagePrivate {
        RepositoryPtr repo;
};

RepositoryClonePage::RepositoryClonePage(RepositoryPtr repo, QWidget* parent) :
    PassthroughPage(parent),
    ui(new Ui::RepositoryClonePage) {
    ui->setupUi(this);
    d = new RepositoryClonePagePrivate();
    d->repo = repo;

    connect(repo.data(), &Repository::stateDescriptionChanged, this, [=] {
        ui->stateDescription->setText(repo->stateDescription());
    });
    connect(repo.data(), &Repository::stateInformationalTextChanged, this, [=] {
        ui->stateInformationalText->setText(repo->stateInformationalText());
    });
    connect(repo.data(), &Repository::stateProgressChanged, this, [=] {
        ui->stateProgress->setVisible(repo->stateProvidesProgress());
        ui->stateProgress->setValue(repo->stateProgress());
        ui->stateProgress->setMaximum(repo->stateTotalProgress());
    });

    ui->stateDescription->setText(repo->stateDescription());
    ui->stateInformationalText->setText(repo->stateInformationalText());
    ui->stateProgress->setVisible(repo->stateProvidesProgress());
    ui->stateProgress->setValue(repo->stateProgress());
    ui->stateProgress->setMaximum(repo->stateTotalProgress());

    connect(repo.data(), &Repository::stateChanged, this, [=] {
        if (repo->state() == Repository::Idle) {
            auto* editor = new ProjectPage(repo->repositoryPath());
            ui->stackedWidget->addWidget(editor);
            ui->stackedWidget->setCurrentWidget(editor);
            this->setPassthroughPage(editor);
        }
    });

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    this->defaultTabButton()->setText(tr("Cloning Repository"));
}

RepositoryClonePage::~RepositoryClonePage() {
    delete ui;
    delete d;
}
