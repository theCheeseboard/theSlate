#ifndef ABSTRACTPAGE_H
#define ABSTRACTPAGE_H

#include <QWidget>
#include <tpromise.h>
#include <Task>

class tWindowTabberButton;
class AbstractPage : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractPage(QWidget* parent = nullptr);

        virtual tWindowTabberButton* tabButton() = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;
        virtual QCoro::Task<> save() = 0;
        virtual QCoro::Task<> saveAs() = 0;
        virtual QCoro::Task<> saveAll() = 0;

        QCoro::Task<> saveAndClose(bool silent);
        virtual QCoro::Task<> saveBeforeClose(bool silent) = 0;
        virtual bool saveAndCloseShouldAskUserConfirmation() = 0;

    signals:
        void done();
};

#endif // ABSTRACTPAGE_H
