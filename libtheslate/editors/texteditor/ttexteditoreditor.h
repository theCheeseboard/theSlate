#ifndef TTEXTEDITOREDITOR_H
#define TTEXTEDITOREDITOR_H

#include "../abstracteditor/abstracteditor.h"
#include <QCoroTask>

class LanguageServerProcess;

struct TextDelta;
struct TTextEditorEditorPrivate;
class TTextEditorEditor : public AbstractEditor {
        Q_OBJECT
    public:
        explicit TTextEditorEditor(QWidget* parent = nullptr);
        ~TTextEditorEditor();

    signals:

    private:
        TTextEditorEditorPrivate* d;

        QCoro::Task<LanguageServerProcess*> languageServer();
        QCoro::Task<> textChanged(QList<TextDelta> deltas);
        QCoro::Task<> updateDiagnostics();

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
