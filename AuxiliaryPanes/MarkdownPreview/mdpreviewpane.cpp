#include "mdpreviewpane.h"
#include "ui_mdpreviewpane.h"

#include <QUrl>
#include <QFile>
#include <QDesktopServices>
#include "webpage.h"

MdPreviewPane::MdPreviewPane(QWidget *parent) :
    AuxiliaryPane(parent),
    ui(new Ui::MdPreviewPane)
{
    ui->setupUi(this);

    view = new QWebEngineView();
    view->setPage(new WebPage());
    this->layout()->addWidget(view);

    QFile f(":/marked/marked.js");
    f.open(QFile::ReadOnly);
    QJSValue v = js.evaluate(f.readAll());
}

MdPreviewPane::~MdPreviewPane()
{
    delete ui;
}

void MdPreviewPane::parseFile(QUrl fileName, QString fileData) {
    QStringList styles;
    styles.append("body {");
    styles.append("color: " + this->palette().color(QPalette::WindowText).name(QColor::HexRgb) + ";");
    styles.append("background-color: " + this->palette().color(QPalette::Window).name(QColor::HexRgb) + ";");
    styles.append("font-family: \"" + this->font().family() + "\",sans-serif;");
    styles.append("}");
    styles.append("a:not([href=\"\"]) {");
    styles.append("color: " + this->palette().color(QPalette::Link).name(QColor::HexRgb) + ";");
    styles.append("}");

    QStringList htmlParts;
    htmlParts.append("<html><head>");
    htmlParts.append("<link rel=\"stylesheet\" href=\"qrc:/marked/styles.css\" />");
    htmlParts.append("<style>" + styles.join("\n") + "</style>");
    htmlParts.append("</head><body>");
    htmlParts.append(js.globalObject().property("marked").call({fileData}).toString());
    htmlParts.append("</body></html>");
    view->setHtml(htmlParts.join(""), fileName);
}

void MdPreviewPane::cursorChanged(QTextCursor cursor) {

}
