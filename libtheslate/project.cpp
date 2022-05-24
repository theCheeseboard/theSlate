#include "project.h"

struct ProjectPrivate {
        QString projectDir;
};

Project::Project(QString projectDir, QObject* parent) :
    QObject{parent} {
    d = new ProjectPrivate();
    d->projectDir = projectDir;
}

Project::~Project() {
    delete d;
}

QString Project::projectDir() {
    return d->projectDir;
}

void Project::reloadProjectConfigurations()
{

}
