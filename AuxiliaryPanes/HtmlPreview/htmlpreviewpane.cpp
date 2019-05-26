#include "htmlpreviewpane.h"
#include "ui_htmlpreviewpane.h"

#include "webpage.h"
#include <QUrl>
#include <QWebEngineSettings>

HtmlPreviewPane::HtmlPreviewPane(QWidget *parent) :
    AuxiliaryPane(parent),
    ui(new Ui::HtmlPreviewPane)
{
    ui->setupUi(this);

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    view = new QWebEngineView();
    view->setPage(new WebPage());
    this->layout()->addWidget(view);
}

HtmlPreviewPane::~HtmlPreviewPane()
{
    delete ui;
}

void HtmlPreviewPane::parseFile(QUrl fileName, QString fileData) {
    view->setHtml(fileData, fileName);
}

void HtmlPreviewPane::cursorChanged(QTextCursor cursor) {

}
