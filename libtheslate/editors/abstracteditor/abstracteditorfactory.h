#ifndef ABSTRACTEDITORFACTORY_H
#define ABSTRACTEDITORFACTORY_H

#include <QObject>

class AbstractEditor;
class AbstractEditorFactory : public QObject {
        Q_OBJECT
    public:
        explicit AbstractEditorFactory(QObject* parent = nullptr);

        virtual AbstractEditor* create() = 0;

    signals:
};

#endif // ABSTRACTEDITORFACTORY_H
