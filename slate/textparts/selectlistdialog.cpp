#include "selectlistdialog.h"
#include "ui_selectlistdialog.h"

#include <QScroller>

struct SelectListDialogPrivate {
    QList<SelectListItem> items;
    QList<SelectListItem> shownItems;
};

SelectListItem::SelectListItem(QString text) {
    this->text = text;
}

SelectListItem::SelectListItem(QString text, QVariant data) {
    this->text = text;
    this->data = data;
}

SelectListDialog::SelectListDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectListDialog)
{
    ui->setupUi(this);
    d = new SelectListDialogPrivate();

#ifdef Q_OS_MAC
    ui->cancelButton->setVisible(false);
    ui->acceptButton->setVisible(false);
#else
    ui->windowControlsMac->setVisible(false);
#endif

    this->setFocusProxy(ui->searchEdit);
    QScroller::grabGesture(ui->listOptions, QScroller::LeftMouseButtonGesture);
}

SelectListDialog::~SelectListDialog()
{
    delete d;
    delete ui;
}

void SelectListDialog::setTitle(QString title) {
    ui->titleLabel->setText(title);
}

void SelectListDialog::setText(QString text) {
    ui->textLabel->setText(text);
}

void SelectListDialog::on_cancelButton_clicked()
{
    emit rejected();
}

void SelectListDialog::on_acceptButton_clicked()
{
    SelectListItem item = d->shownItems.at(ui->listOptions->currentRow());
    emit accepted(item.data);
}

void SelectListDialog::setItems(QList<SelectListItem> items) {
    d->items = items;
    d->shownItems = items;

    ui->listOptions->clear();
    for (SelectListItem item : items) {
        QListWidgetItem* i = new QListWidgetItem();
        i->setText(item.text);
        i->setIcon(item.icon);
        ui->listOptions->addItem(i);
    }
}

void SelectListDialog::on_searchEdit_textChanged(const QString &arg1)
{
    if (arg1 == "") {
        setItems(d->items);
    } else {
        ui->listOptions->clear();
        d->shownItems.clear();
        for (SelectListItem item : d->items) {
            QStringList searches = item.tags;
            searches.append(item.text);

            //Start searching all tags and language name
            for (QString search : searches) {
                if (search.contains(arg1, Qt::CaseInsensitive)) {
                    QListWidgetItem* i = new QListWidgetItem();
                    i->setText(item.text);
                    i->setIcon(item.icon);
                    ui->listOptions->addItem(i);

                    d->shownItems.append(item);
                    break;
                }
            }
        }

        if (ui->listOptions->count() > 0) {
            ui->listOptions->setCurrentRow(0);
        }
    }
}

void SelectListDialog::on_listOptions_currentRowChanged(int currentRow)
{
    if (currentRow == -1) {
        ui->acceptButton->setEnabled(false);
        ui->macAcceptButton->setEnabled(false);
    } else {
        ui->acceptButton->setEnabled(true);
        ui->macAcceptButton->setEnabled(true);
    }
}

void SelectListDialog::on_searchEdit_returnPressed()
{
    if (ui->acceptButton->isEnabled()) ui->acceptButton->click();
}

void SelectListDialog::on_listOptions_activated(const QModelIndex &index)
{
    SelectListItem item = d->shownItems.at(index.row());
    emit accepted(item.data);
}
