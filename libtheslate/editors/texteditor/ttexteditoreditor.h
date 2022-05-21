#ifndef TTEXTEDITOREDITOR_H
#define TTEXTEDITOREDITOR_H

#include "../abstracteditor/abstracteditor.h"

struct TTextEditorEditorPrivate;
class TTextEditorEditor : public AbstractEditor {
        Q_OBJECT
    public:
        explicit TTextEditorEditor(QWidget* parent = nullptr);
        ~TTextEditorEditor();

    signals:

    private:
        TTextEditorEditorPrivate* d;

        // AbstractEditor interface
    public:
        void undo();
        void redo();
        void setData(QByteArray data);
        QByteArray data();
        void setCurrentUrl(QUrl url);
        QUrl currentUrl();
        bool haveUnsavedChanges();
        void setChangesSaved();

        // AbstractEditor interface
public:
        QStringList nameFilters();
        QString defaultExtension();
};

#endif // TTEXTEDITOREDITOR_H
