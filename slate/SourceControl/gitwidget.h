#ifndef GITWIDGET_H
#define GITWIDGET_H

#include <QWidget>
#include <QUrl>
#include <tpromise.h>
#include <functional>

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

        void commit();
        void pull(QString from = "");
        void push(QString to = "");

    private slots:
        void on_branchesButton_toggled(bool checked);

        void on_commitsButton_toggled(bool checked);

        void on_branchesList_customContextMenuRequested(const QPoint &pos);

        void on_branchesList_activated(const QModelIndex &index);

        void on_logList_customContextMenuRequested(const QPoint &pos);

        void on_logList_activated(const QModelIndex &index);

        void on_initGitButton_clicked();

    private:
        Ui::GitWidget *ui;

        GitWidgetPrivate* d;
        void setAuthenticationDetails(QString message, std::function<void()> callback);
        void showConflictDialog(QString operation);
        void merge(QString other);
};

#endif // GITWIDGET_H
