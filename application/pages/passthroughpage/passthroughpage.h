#ifndef PASSTHROUGHPAGE_H
#define PASSTHROUGHPAGE_H

#include "../abstractpage/abstractpage.h"

struct PassthroughPagePrivate;
class PassthroughPage : public AbstractPage {
        Q_OBJECT
    public:
        explicit PassthroughPage(QWidget* parent = nullptr);
        ~PassthroughPage();

    protected:
        void setPassthroughPage(AbstractPage* page);
        tWindowTabberButton* defaultTabButton();

    signals:

    private:
        PassthroughPagePrivate* d;

        void copyCurrentTabButton();

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

#endif // PASSTHROUGHPAGE_H
