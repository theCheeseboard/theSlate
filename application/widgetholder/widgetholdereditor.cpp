#include "widgetholdereditor.h"
#include "ui_widgetholdereditor.h"

#include <QUuid>

struct WidgetHolderEditorPrivate {
        static QMap<QUuid, QWidget*> widgetUrls;
};

QMap<QUuid, QWidget*> WidgetHolderEditorPrivate::widgetUrls = QMap<QUuid, QWidget*>();

WidgetHolderEditor::WidgetHolderEditor(QWidget* parent) :
    AbstractEditor(parent),
    ui(new Ui::WidgetHolderEditor) {
    ui->setupUi(this);
    d = new WidgetHolderEditorPrivate();
}

WidgetHolderEditor::~WidgetHolderEditor() {
    delete ui;
    delete d;
}

QUrl WidgetHolderEditor::urlForWidget(QWidget* widget) {
    QUrl url;
    url.setScheme("editor");
    url.setHost("theslate");

    auto uuid = QUuid::createUuid();
    url.setPath("/" + uuid.toString());

    WidgetHolderEditorPrivate::widgetUrls.insert(uuid, widget);
    return url;
}

void WidgetHolderEditor::undo() {
}

void WidgetHolderEditor::redo() {
}

void WidgetHolderEditor::setData(QByteArray data) {
    QUrl url(data);
    if (url.scheme() != "editor") return;
    if (url.host() != "theslate") return;

    auto uuid = QUuid::fromString(url.path().remove('/'));
    auto* widget = d->widgetUrls.value(uuid);
    ui->widgetLayout->addWidget(widget);

    // TODO: maybe remove the widget from the widget URLs?
}

QByteArray WidgetHolderEditor::data() {
    return QByteArray();
}

bool WidgetHolderEditor::haveUnsavedChanges() {
    return false;
}

void WidgetHolderEditor::setChangesSaved() {
}

QStringList WidgetHolderEditor::nameFilters() {
    return {};
}

QString WidgetHolderEditor::defaultExtension() {
    return "";
}
