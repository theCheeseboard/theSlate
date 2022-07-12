#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <QObject>
#include "libtheslate_global.h"

class EditorManager;
class BuildEngineManager;
class LogScannerManager;

struct StateManagerPrivate;
class LIBTHESLATE_EXPORT StateManager : public QObject {
        Q_OBJECT
    public:
        ~StateManager();

        static StateManager* instance();
        static EditorManager* editor();
        static BuildEngineManager* buildEngine();
        static LogScannerManager* logScanner();

    signals:

    private:
        explicit StateManager(QObject* parent = nullptr);
        StateManagerPrivate* d;
};

#endif // STATEMANAGER_H
