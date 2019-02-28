#ifndef GITWIDGET_H
#define GITWIDGET_H

#include <QWidget>
#include <QUrl>

namespace Ui {
class GitWidget;
}

struct GitWidgetPrivate;
class GitWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit GitWidget(QWidget *parent = nullptr);
        ~GitWidget();

    public slots:
        void setCurrentDocument(QUrl currentDocument);
        void updateStatus();

    private:
        Ui::GitWidget *ui;

        GitWidgetPrivate* d;
};

#endif // GITWIDGET_H
