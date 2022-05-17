#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#include <QObject>

class AbstractEditor;
class AbstractEditorFactory;
struct EditorManagerPrivate;
class EditorManager : public QObject {
        Q_OBJECT
    public:
        explicit EditorManager(QObject* parent = nullptr);
        ~EditorManager();

        AbstractEditor* createEditor(QString editorType);
        QString editorTypeForFileName(QString fileName);

        void registerFactory(QString name, AbstractEditorFactory* factory);

    signals:

    private:
        EditorManagerPrivate* d;
};

#endif // EDITORMANAGER_H
