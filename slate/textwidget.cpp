#include "textwidget.h"
#include "ui_textwidget.h"

#include "plugins/pluginmanager.h"

extern PluginManager* plugins;

struct TextWidgetPrivate {
    MainWindow* parent;

    QList<AuxiliaryPane*> auxPanes;
    QTimer* editedTimer;
};

TextWidget::TextWidget(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::TextWidget)
{
    ui->setupUi(this);
    d = new TextWidgetPrivate();
    d->parent = parent;

    ui->auxEditors->setVisible(false);
    ui->editor->setMainWindow(parent);
    ui->editor->setStatusBar(ui->statusBar);
    ui->findReplaceWidget->setEditor(ui->editor);
    ui->statusBar->setEditor(ui->editor);

    ui->findReplaceWidget->hide();

    d->editedTimer = new QTimer(this);
    d->editedTimer->setInterval(500);
    d->editedTimer->setSingleShot(true);
    connect(d->editedTimer, &QTimer::timeout, this, [=] {
        //Tell all the aux panes that there are edits
        for (AuxiliaryPane* auxPane : d->auxPanes) {
            auxPane->parseFile(ui->editor->fileUrl(), ui->editor->toPlainText());
        }
    });
    connect(ui->editor, &TextEditor::textChanged, this, [=] {
        if (d->editedTimer->isActive()) d->editedTimer->stop();
        d->editedTimer->start();
    });
    connect(ui->editor, &TextEditor::backendChanged, this, [=] {
        updateAuxMenu();
    });

    updateAuxMenu();
}

TextWidget::~TextWidget()
{
    delete d;
    delete ui;
}

TextEditor* TextWidget::editor() {
    return ui->editor;
}

void TextWidget::showFindReplace() {
    ui->findReplaceWidget->show();
    ui->findReplaceWidget->setFocus();
}

void TextWidget::openAuxPane(AuxiliaryPane *pane) {
    d->auxPanes.append(pane);
    pane->setVisible(true);

    ui->auxEditors->addTab(pane, pane->windowTitle());
    ui->auxEditors->setVisible(true);

    connect(this, &TextWidget::destroyed, pane, &AuxiliaryPane::deleteLater);
    connect(pane, &AuxiliaryPane::windowTitleChanged, this, [=](QString title) {
        ui->auxEditors->setTabText(ui->auxEditors->indexOf(pane), title);
    });
    connect(ui->editor, &TextEditor::cursorPositionChanged, this, [=] {
        pane->cursorChanged(ui->editor->textCursor());
    });

    pane->parseFile(ui->editor->fileUrl(), ui->editor->toPlainText());
}

void TextWidget::on_auxEditors_tabCloseRequested(int index)
{
    AuxiliaryPane* pane = static_cast<AuxiliaryPane*>(ui->auxEditors->widget(index));
    ui->auxEditors->removeTab(index);

    if (ui->auxEditors->count() == 0) {
        ui->auxEditors->setVisible(false);
    }
}

void TextWidget::updateAuxMenu() {
    QList<AuxiliaryPaneCapabilities> panes = plugins->auxiliaryPanesForUrl(ui->editor->fileUrl());
    if (panes.count() == 0) {
        ui->statusBar->setAuxMenu(nullptr);
    } else {
        QMenu* menu = new QMenu();
        menu->addSection(tr("Supported Auxiliary Panes"));
        for (AuxiliaryPaneCapabilities pane : panes) {
            menu->addAction(pane.name, [=] {
                openAuxPane(pane.makePane());
            });
        }
        ui->statusBar->setAuxMenu(menu);
    }
}
