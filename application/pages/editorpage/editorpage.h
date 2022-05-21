#ifndef EDITORPAGE_H
#define EDITORPAGE_H

#include "../abstractpage/abstractpage.h"

namespace Ui {
    class EditorPage;
}

class AbstractEditor;
struct EditorPagePrivate;
class EditorPage : public AbstractPage {
        Q_OBJECT

    public:
        explicit EditorPage(QString editorType = "text", QWidget* parent = nullptr);
        ~EditorPage();

        void undo();
        void redo();

        tWindowTabberButton* tabButton();
        AbstractEditor* editor();

    private:
        Ui::EditorPage* ui;
        EditorPagePrivate* d;

        void saveToFile(QUrl url);

        // AbstractPage interface
    public:
        tPromise<void>* save();
        tPromise<void>* saveAs();
        tPromise<void>* saveAll();
        tPromise<void>* saveBeforeClose(bool silent);
        bool saveAndCloseShouldAskUserConfirmation();
};

#endif // EDITORPAGE_H
