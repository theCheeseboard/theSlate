#include "editorpage.h"
#include "ui_editorpage.h"

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

tPromise<void>* EditorPage::save() {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        if (d->editor->currentUrl().isEmpty()) {
            this->saveAs()->then([=] {
                              res();
                          })
                ->error([=](QString error) {
                    rej(error);
                });
        } else {
            this->saveToFile(d->editor->currentUrl());
            res();
        }
    });
}

tPromise<void>* EditorPage::saveAs() {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        QFileDialog* fileDialog = new QFileDialog(this);
        fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        fileDialog->setFileMode(QFileDialog::AnyFile);
        fileDialog->setNameFilters(d->editor->nameFilters());
        fileDialog->setDefaultSuffix(d->editor->defaultExtension());
        connect(fileDialog, &QFileDialog::finished, this, [=](int result) {
            if (result == QFileDialog::Accepted) {
                this->saveToFile(QUrl::fromLocalFile(fileDialog->selectedFiles().first()));
                res();
            } else {
                rej("User cancelled");
            }
        });
        connect(fileDialog, &QFileDialog::finished, fileDialog, &QFileDialog::deleteLater);
        fileDialog->open();
    });
}

tPromise<void>* EditorPage::saveAll() {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        if (d->editor->currentUrl().isEmpty()) {
            this->saveToFile(d->editor->currentUrl());
            res();
        } else {
            rej("User cancelled");
        }
    });
}

void EditorPage::saveAndClose(bool silent) {
    if (!d->editor) {
        emit done();
        return;
    }

    if (!d->editor->haveUnsavedChanges()) {
        emit done();
        return;
    }

    if (silent) {
        if (d->editor->currentUrl().isEmpty()) {
            // Discard these changes
            emit done();
            return;
        } else {
            this->save()->then([=] {
                emit done();
                return;
            });
        }
    }

    tMessageBox* box = new tMessageBox(this->window());
    if (d->editor->currentUrl().isEmpty()) {
        box->setTitleBarText(tr("Save changes?"));
    } else {
        box->setTitleBarText(tr("Save changes to %1?").arg(d->editor->currentUrl().fileName()));
    }
    box->setMessageText(tr("Do you want to save the changes you made to this file?"));
    box->setInformativeText(tr("If you don't save this document, any changes will be lost forever."));

    tMessageBoxButton* saveButton;
    if (d->editor->currentUrl().isEmpty()) {
        saveButton = box->addButton(tr("Save As..."), QMessageBox::AcceptRole);
    } else {
        saveButton = box->addButton(tr("Save"), QMessageBox::AcceptRole);
    }
    connect(saveButton, &tMessageBoxButton::buttonPressed, this, [=] {
        this->save()->then([=] {
            emit done();
            return;
        });
    });

    tMessageBoxButton* discardButton = box->addStandardButton(QMessageBox::Discard);
    connect(discardButton, &tMessageBoxButton::buttonPressed, this, [=] {
        emit done();
        return;
    });

    tMessageBoxButton* cancelButton = box->addStandardButton(QMessageBox::Cancel);
    box->show(true);
    return;
}

bool EditorPage::saveAndCloseShouldAskUserConfirmation() {
    if (d->editor && d->editor->haveUnsavedChanges()) return true;
    return false;
}
