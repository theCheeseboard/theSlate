#ifndef BUILDJOBPAGEWIDGET_H
#define BUILDJOBPAGEWIDGET_H

#include <QWidget>
#include <project/buildjob.h>

namespace Ui {
    class BuildJobPageWidget;
}

struct BuildJobPageWidgetPrivate;
class BuildJobPageWidget : public QWidget {
        Q_OBJECT

    public:
        explicit BuildJobPageWidget(BuildJobPtr buildJob, QWidget* parent = nullptr);
        ~BuildJobPageWidget();

        void appendToLog(QString contents);

    signals:
        void requestFileOpen(QUrl url);

    private slots:
        void on_issuesList_clicked(const QModelIndex& index);

    private:
        Ui::BuildJobPageWidget* ui;
        BuildJobPageWidgetPrivate* d;
};

#endif // BUILDJOBPAGEWIDGET_H
