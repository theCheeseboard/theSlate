#ifndef BUILDJOB_H
#define BUILDJOB_H

#include <QObject>

struct BuildJobPrivate;
class BuildJob : public QObject {
        Q_OBJECT
    public:
        explicit BuildJob(QObject* parent = nullptr);
        ~BuildJob();

        struct BuildIssue {
                enum Type {
                    Informational,
                    Warning,
                    Error
                };

                Type issueType;
                QString message;

                QString file;
                int line;
                int col;
        };

        enum State {
            Running,
            Failed,
            Successful
        };

        State state();

        QString title();
        QString description();
        int progress();
        int maxProgress();
        int step();
        int maxStep();

        QString buildLog();
        QList<BuildIssue> buildIssues();
        int countIssues(BuildIssue::Type issueType);

        QDateTime buildStartDate();

        virtual void start() = 0;

    protected:
        void setState(State state);
        void setTitle(QString title);
        void setDescription(QString description);

        void setProgress(int progress);
        void setMaxProgress(int maxProgress);
        void setStep(int step);
        void setMaxStep(int maxStep);

        void appendToBuildLog(QString buildLog);
        void appendToBuildIssues(BuildIssue issue);

    signals:
        void stateChanged(BuildJob::State state);
        void titleChanged(QString title);
        void descriptionChanged(QString description);

        void progressChanged(int progress, int maxProgress);
        void stepChanged(int step, int maxStep);

        void buildLogAppendedTo(QString appendedContents);
        void buildIssuesAppendedTo(BuildIssue issue);

    private:
        BuildJobPrivate* d;
};

typedef QSharedPointer<BuildJob> BuildJobPtr;

#endif // BUILDJOB_H
