#include "textmergepopover.h"
#include "ui_textmergepopover.h"

TextMergePopover::TextMergePopover(QString file1, QString file2, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TextMergePopover) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->mergeTool->loadDiff(file1, file2);

    connect(ui->mergeTool, &TextMergeTool::conflictResolutionCompletedChanged, this, [this] {
        ui->acceptButton->setEnabled(ui->mergeTool->isConflictResolutionCompleted());
    });
    ui->acceptButton->setEnabled(ui->mergeTool->isConflictResolutionCompleted());
}

TextMergePopover::~TextMergePopover() {
    delete ui;
}

void TextMergePopover::on_titleLabel_backButtonClicked() {
    emit finished(false, "");
}

void TextMergePopover::on_acceptButton_clicked() {
    emit finished(true, ui->mergeTool->conflictResolution());
}
