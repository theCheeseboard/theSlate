#include "editorpage.h"
#include "textmergepopover.h"
#include "ui_editorpage.h"

#include <QCoroSignal>
#include <QFile>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <editormanager.h>
#include <editors/abstracteditor/abstracteditor.h>
#include <statemanager.h>
#include <tmessagebox.h>
#include <tpopover.h>
#include <twindowtabberbutton.h>

struct EditorPagePrivate {
        tWindowTabberButton* tabButton;
        QString editorType;
        AbstractEditor* editor = nullptr;

        QFileSystemWatcher watcher;
        bool haveSaveConflict = false;
};

EditorPage::EditorPage(QString editorType, QWidget* parent) :
    AbstractPage(parent),
    ui(new Ui::EditorPage) {
    ui->setupUi(this);

    d = new EditorPagePrivate;
    d->tabButton = new tWindowTabberButton();
    d->tabButton->setText(tr("Empty Document"));
    d->editorType = editorType;

    d->editor = StateManager::editor()->createEditor(editorType);
    if (d->editor) {
        ui->stackedWidget->addWidget(d->editor);
        ui->stackedWidget->setCurrentWidget(d->editor, false);

        connect(d->editor, &AbstractEditor::currentFileChanged, this, [=](QUrl currentFile) {
            d->tabButton->setText(currentFile.fileName());
        });
    }

    connect(&d->watcher, &QFileSystemWatcher::fileChanged, this, &EditorPage::fileChanged);
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

    this->setEditorUrl(file);
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

QCoro::Task<> EditorPage::saveToFile(QUrl url) {
    if (d->haveSaveConflict && url == d->editor->currentUrl()) {
        tMessageBox box(this->window());
        box.setTitleBarText(tr("Save Conflict"));
        box.setMessageText(tr("The file on disk has been changed. What would you like to do?"));
        box.setInformativeText(tr("If you overwrite the file, the file on disk will be lost forever."));

        tMessageBoxButton* mergeButton;
        if (d->editorType == "text") mergeButton = box.addButton(tr("Merge Changes"), QMessageBox::AcceptRole);
        tMessageBoxButton* overwriteButton = box.addButton(tr("Overwrite"), QMessageBox::DestructiveRole);
        tMessageBoxButton* discardButton = box.addButton(tr("Discard Changes"), QMessageBox::DestructiveRole);
        tMessageBoxButton* cancelButton = box.addStandardButton(QMessageBox::Cancel);

        auto button = co_await box.presentAsync();
        if (button == mergeButton) {
            bool ok;
            QString resolution;

            QFile f(url.toLocalFile());
            f.open(QFile::ReadOnly);

            TextMergePopover* jp = new TextMergePopover(f.readAll(), d->editor->data());
            f.close();

            tPopover popover(jp);
            popover.setPopoverWidth(SC_DPI_W(-200, this));
            popover.setPopoverSide(tPopover::Bottom);
            connect(jp, &TextMergePopover::finished, this, [&ok, &resolution, &popover](bool popoverOk, QString popoverResolution) {
                ok = popoverOk;
                resolution = popoverResolution;
                popover.dismiss();
            });
            popover.show(this->window());

            co_await qCoro(&popover, &tPopover::dismissed);

            if (!ok) {
                throw QException();
            } else {
                d->editor->setData(resolution.toUtf8());
            }
        } else if (button == discardButton) {
            this->discardContentsAndOpenFile(url);
            co_return;
        } else if (button == cancelButton) {
            throw QException();
        }
    }

    // TODO: Error handling
    QSignalBlocker blocker(d->watcher);
    QFile file(url.toLocalFile());
    file.open(QFile::WriteOnly);
    file.write(d->editor->data());
    file.close();

    this->setEditorUrl(url);

    co_return;
}

void EditorPage::fileChanged() {
    if (d->editor->haveUnsavedChanges()) {
        // TODO: Show save conflict banner
        d->haveSaveConflict = true;
    } else {
        auto file = d->editor->currentUrl().toLocalFile();
        if (!d->watcher.files().contains(file)) {
            if (QFile::exists(file)) {
                this->discardContentsAndOpenFile(d->editor->currentUrl());
                d->watcher.addPath(file);
            } else {
                QTimer::singleShot(1000, [=] {
                    if (QFile::exists(file)) {
                        this->discardContentsAndOpenFile(d->editor->currentUrl());
                        d->watcher.addPath(file);
                    }
                });
            }
        } else {
            this->discardContentsAndOpenFile(d->editor->currentUrl());
        }
    }
}

void EditorPage::setEditorUrl(QUrl url) {
    // TODO: Remove save conflict banner if needed
    d->editor->setCurrentUrl(url);
    d->haveSaveConflict = false;
    d->watcher.removePaths(d->watcher.files());
    d->watcher.addPath(url.toLocalFile());
}

QCoro::Task<> EditorPage::save() {
    if (d->editor->currentUrl().isEmpty()) {
        co_await this->saveAs();
    } else {
        co_await this->saveToFile(d->editor->currentUrl());
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
        co_await this->saveToFile(d->editor->currentUrl());
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
