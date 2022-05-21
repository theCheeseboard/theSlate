#ifndef ABSTRACTPAGE_H
#define ABSTRACTPAGE_H

#include <QWidget>
#include <tpromise.h>

class tWindowTabberButton;
class AbstractPage : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractPage(QWidget* parent = nullptr);

        virtual tWindowTabberButton* tabButton() = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;
        virtual tPromise<void>* save() = 0;
        virtual tPromise<void>* saveAs() = 0;
        virtual tPromise<void>* saveAll() = 0;

        void saveAndClose(bool silent);
        virtual tPromise<void>* saveBeforeClose(bool silent) = 0;
        virtual bool saveAndCloseShouldAskUserConfirmation() = 0;

    signals:
        void done();
};

#endif // ABSTRACTPAGE_H
