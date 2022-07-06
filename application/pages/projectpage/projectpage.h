#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include "../abstractpage/abstractpage.h"

namespace Ui {
    class ProjectPage;
}

class AbstractLeftPane;
struct ProjectPagePrivate;
class ProjectPage : public AbstractPage {
        Q_OBJECT

    public:
        explicit ProjectPage(QString projectDirectory, QWidget* parent = nullptr);
        ~ProjectPage();

        void addLeftPaneItem(AbstractLeftPane* leftPane);

    private:
        Ui::ProjectPage* ui;
        ProjectPagePrivate* d;

        void openUrl(QUrl url);

        // AbstractPage interface
    public:
        tWindowTabberButton* tabButton();
        void undo();
        void redo();
        QCoro::Task<> save();
        QCoro::Task<> saveAs();
        QCoro::Task<> saveAll();
        QCoro::Task<> saveBeforeClose(bool silent);
        bool saveAndCloseShouldAskUserConfirmation();
};

#endif // PROJECTPAGE_H
