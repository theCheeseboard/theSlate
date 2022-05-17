#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <QObject>

class EditorManager;

struct StateManagerPrivate;
class StateManager : public QObject {
        Q_OBJECT
    public:
        ~StateManager();

        static StateManager* instance();
        static EditorManager* editor();

    signals:

    private:
        explicit StateManager(QObject* parent = nullptr);
        StateManagerPrivate* d;
};

#endif // STATEMANAGER_H
