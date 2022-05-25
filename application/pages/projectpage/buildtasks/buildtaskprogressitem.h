#ifndef BUILDTASKPROGRESSITEM_H
#define BUILDTASKPROGRESSITEM_H

#include <QWidget>
#include <project/buildjob.h>

namespace Ui {
    class BuildTaskProgressItem;
}

struct BuildTaskProgressItemPrivate;
class BuildTaskProgressItem : public QWidget {
        Q_OBJECT

    public:
        explicit BuildTaskProgressItem(BuildJobPtr buildJob, QWidget* parent = nullptr);
        ~BuildTaskProgressItem();

        void updateTitle(QString title);
        void updateDescription(QString description);
        void updateProgress(int progress, int maxProgress);
        void updateSteps(int steps, int maxSteps);

        void stateChanged(BuildJob::State state);

    signals:
        void updateHeight();
        void done();

    private:
        Ui::BuildTaskProgressItem* ui;
        BuildTaskProgressItemPrivate* d;

        void animateOut();

        // QWidget interface
protected:
        void paintEvent(QPaintEvent *event);
};

#endif // BUILDTASKPROGRESSITEM_H
