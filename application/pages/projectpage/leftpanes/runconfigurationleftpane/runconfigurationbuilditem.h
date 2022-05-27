#ifndef RUNCONFIGURATIONBUILDITEM_H
#define RUNCONFIGURATIONBUILDITEM_H

#include <QWidget>
#include <project/buildjob.h>

namespace Ui {
    class RunConfigurationBuildItem;
}

struct RunConfigurationBuildItemPrivate;
class RunConfigurationBuildItem : public QWidget {
        Q_OBJECT

    public:
        explicit RunConfigurationBuildItem(BuildJobPtr buildJob, QWidget* parent = nullptr);
        ~RunConfigurationBuildItem();

        void setTitle(QString title);
        void setState(BuildJob::State state);

    signals:
        void requestFileOpen(QUrl url);

    private:
        Ui::RunConfigurationBuildItem* ui;
        RunConfigurationBuildItemPrivate* d;

        // QWidget interface
    protected:
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void enterEvent(QEnterEvent* event);
        void leaveEvent(QEvent* event);
};

#endif // RUNCONFIGURATIONBUILDITEM_H
