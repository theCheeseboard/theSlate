#ifndef WIDGETHOLDEREDITOR_H
#define WIDGETHOLDEREDITOR_H

#include <editors/abstracteditor/abstracteditor.h>

namespace Ui {
    class WidgetHolderEditor;
}

struct WidgetHolderEditorPrivate;
class WidgetHolderEditor : public AbstractEditor {
        Q_OBJECT

    public:
        explicit WidgetHolderEditor(QWidget* parent = nullptr);
        ~WidgetHolderEditor();

        static QUrl urlForWidget(QWidget* widget);

    private:
        Ui::WidgetHolderEditor* ui;
        WidgetHolderEditorPrivate* d;

        // AbstractEditor interface
    public:
        void undo();
        void redo();
        void setData(QByteArray data);
        QByteArray data();
        bool haveUnsavedChanges();
        void setChangesSaved();
        QStringList nameFilters();
        QString defaultExtension();
};

#endif // WIDGETHOLDEREDITOR_H
