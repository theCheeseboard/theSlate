#include "editorpage.h"
#include "ui_editorpage.h"

#include <QCoroSignal>
#include <QFile>
#include <QFileDialog>
#include <editormanager.h>
#include <editors/abstracteditor/abstracteditor.h>
#include <statemanager.h>
#include <tmessagebox.h>
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

        connect(d->editor, &AbstractEditor::currentFileChanged, this, [=](QUrl currentFile) {
            d->tabButton->setText(currentFile.fileName());
        });
    }
}

EditorPage::~EditorPage() {
    delete ui;
    delete d;
}

void EditorPage::setProject(ProjectPtr project) {
    if (d->editor) d->editor->setProject(project);
}

void EditorPage::discardContentsAndOpenFile(QUrl file) {
    if (!d->editor) return;

    if (file.scheme() == "editor") {
        d->editor->setData(file.toString().toUtf8());
    } else {
        QFile f(file.toLocalFile());
        f.open(QFile::ReadOnly);
        d->editor->setData(f.readAll());
        f.close();
    }

    d->editor->setCurrentUrl(file);
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

AbstractEditor* EditorPage::editor() {
    return d->editor;
}

void EditorPage::saveToFile(QUrl url) {
    // TODO: Error handling
    QFile file(url.toLocalFile());
    file.open(QFile::WriteOnly);
    file.write(d->editor->data());
    file.close();

    d->editor->setCurrentUrl(url);
}

QCoro::Task<> EditorPage::save() {
    if (d->editor->currentUrl().isEmpty()) {
        co_await this->saveAs();
    } else {
        this->saveToFile(d->editor->currentUrl());
    }
}

QCoro::Task<> EditorPage::saveAs() {
    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setNameFilters(d->editor->nameFilters());
    fileDialog->setDefaultSuffix(d->editor->defaultExtension());
    fileDialog->open();

    auto result = co_await qCoro(fileDialog, &QFileDialog::finished);
    fileDialog->deleteLater();
    if (result == QFileDialog::Accepted) {
        this->saveToFile(QUrl::fromLocalFile(fileDialog->selectedFiles().first()));
    } else {
        throw QException();
    }
}

QCoro::Task<> EditorPage::saveAll() {
    if (!d->editor->currentUrl().isEmpty()) {
        this->saveToFile(d->editor->currentUrl());
        co_return;
    } else {
        throw QException();
    }
}

QCoro::Task<> EditorPage::saveBeforeClose(bool silent) {
    if (!d->editor) co_return;

    if (!d->editor->haveUnsavedChanges()) co_return;

    if (silent) {
        if (d->editor->currentUrl().isEmpty()) {
            // Discard these changes
            co_return;
        } else {
            co_await this->save();
            co_return;
        }
    }

    tMessageBox box(this->window());
    if (d->editor->currentUrl().isEmpty()) {
        box.setTitleBarText(tr("Save changes?"));
    } else {
        box.setTitleBarText(tr("Save changes to %1?").arg(d->editor->currentUrl().fileName()));
    }
    box.setMessageText(tr("Do you want to save the changes you made to this file?"));
    box.setInformativeText(tr("If you don't save this document, any changes will be lost forever."));

    tMessageBoxButton* saveButton;
    if (d->editor->currentUrl().isEmpty()) {
        saveButton = box.addButton(tr("Save As..."), QMessageBox::AcceptRole);
    } else {
        saveButton = box.addButton(tr("Save"), QMessageBox::AcceptRole);
    }
    tMessageBoxButton* discardButton = box.addStandardButton(QMessageBox::Discard);
    tMessageBoxButton* cancelButton = box.addStandardButton(QMessageBox::Cancel);

    auto button = co_await box.presentAsync();
    if (button == saveButton) {
        co_await this->save();
    } else if (button == cancelButton) {
        throw QException();
    }
}

bool EditorPage::saveAndCloseShouldAskUserConfirmation() {
    if (d->editor && d->editor->haveUnsavedChanges()) return true;
    return false;
}
