#ifndef ABSTRACTEDITORFACTORY_H
#define ABSTRACTEDITORFACTORY_H

#include <QObject>
#include "libtheslate_global.h"

class AbstractEditor;
class LIBTHESLATE_EXPORT AbstractEditorFactory : public QObject {
        Q_OBJECT
    public:
        explicit AbstractEditorFactory(QObject* parent = nullptr);

        virtual AbstractEditor* create() = 0;
        virtual bool canOpen(QUrl url) = 0;

    signals:
};

#endif // ABSTRACTEDITORFACTORY_H
