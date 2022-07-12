#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#include <QObject>
#include "libtheslate_global.h"

class AbstractEditor;
class AbstractEditorFactory;
struct EditorManagerPrivate;
class LIBTHESLATE_EXPORT EditorManager : public QObject {
        Q_OBJECT
    public:
        explicit EditorManager(QObject* parent = nullptr);
        ~EditorManager();

        AbstractEditor* createEditor(QString editorType);
        QString editorTypeForUrl(QUrl url);

        void registerFactory(QString name, AbstractEditorFactory* factory);

    signals:

    private:
        EditorManagerPrivate* d;
};

#endif // EDITORMANAGER_H
