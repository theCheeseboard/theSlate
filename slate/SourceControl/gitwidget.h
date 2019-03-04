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

private slots:

        void on_branchesButton_toggled(bool checked);

        void on_commitsButton_toggled(bool checked);

        void on_branchesList_customContextMenuRequested(const QPoint &pos);

        void on_branchesList_activated(const QModelIndex &index);

        void on_logList_customContextMenuRequested(const QPoint &pos);

        void on_logList_activated(const QModelIndex &index);

private:
        Ui::GitWidget *ui;

        GitWidgetPrivate* d;
};

#endif // GITWIDGET_H
