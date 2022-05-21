#ifndef ABSTRACTEDITOR_H
#define ABSTRACTEDITOR_H

#include <QUrl>
#include <QWidget>

class TextEditorColorScheme;
struct AbstractEditorPrivate;
class AbstractEditor : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractEditor(QWidget* parent = nullptr);
        ~AbstractEditor();

        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual void setData(QByteArray data) = 0;
        virtual QByteArray data() = 0;
        virtual void setCurrentUrl(QUrl url);
        virtual QUrl currentUrl();
        virtual bool haveUnsavedChanges() = 0;
        virtual void setChangesSaved() = 0;

        virtual QStringList nameFilters() = 0;
        virtual QString defaultExtension() = 0;

        TextEditorColorScheme* colorScheme();
        void setColorScheme(QString scheme);

    signals:
        void currentFileChanged(QUrl currentFile);
        void unsavedChangesChanged();

    private:
        AbstractEditorPrivate* d;
};

#endif // ABSTRACTEDITOR_H
