#ifndef EDITORPAGE_H
#define EDITORPAGE_H

#include "../abstractpage/abstractpage.h"

namespace Ui {
    class EditorPage;
}

struct EditorPagePrivate;
class EditorPage : public AbstractPage {
        Q_OBJECT

    public:
        explicit EditorPage(QString editorType = "text", QWidget* parent = nullptr);
        ~EditorPage();

        void undo();
        void redo();

        tWindowTabberButton* tabButton();

    private:
        Ui::EditorPage* ui;
        EditorPagePrivate* d;
};

#endif // EDITORPAGE_H
