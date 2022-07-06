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

        void discardContentsAndOpenFile(QUrl file);

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
        QCoro::Task<> save();
        QCoro::Task<> saveAs();
        QCoro::Task<> saveAll();
        QCoro::Task<> saveBeforeClose(bool silent);
        bool saveAndCloseShouldAskUserConfirmation();
};

#endif // EDITORPAGE_H
