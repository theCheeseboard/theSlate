#ifndef BUILDTASKSCONTAINER_H
#define BUILDTASKSCONTAINER_H

#include "project.h"
#include <QWidget>

namespace Ui {
    class BuildTasksContainer;
}

struct BuildTasksContainerPrivate;
class BuildTasksContainer : public QWidget {
        Q_OBJECT

    public:
        explicit BuildTasksContainer(QWidget* parent = nullptr);
        ~BuildTasksContainer();

        void setProject(ProjectPtr project);

        void addBuildJob(BuildJobPtr buildJob);

        void updateHeight();

    private:
        Ui::BuildTasksContainer* ui;
        BuildTasksContainerPrivate* d;
};

#endif // BUILDTASKSCONTAINER_H
