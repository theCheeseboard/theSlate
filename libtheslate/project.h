#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>

struct ProjectPrivate;
class Project : public QObject {
        Q_OBJECT
    public:
        explicit Project(QString projectDir, QObject* parent = nullptr);
        ~Project();

        QString projectDir();

        void reloadProjectConfigurations();

    signals:

    private:
        ProjectPrivate* d;
};

typedef QSharedPointer<Project> ProjectPtr;

#endif // PROJECT_H
