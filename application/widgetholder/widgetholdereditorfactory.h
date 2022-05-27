#ifndef WIDGETHOLDEREDITORFACTORY_H
#define WIDGETHOLDEREDITORFACTORY_H

#include <editors/abstracteditor/abstracteditorfactory.h>

class WidgetHolderEditorFactory : public AbstractEditorFactory {
        Q_OBJECT
    public:
        explicit WidgetHolderEditorFactory(QObject* parent = nullptr);

    signals:

        // AbstractEditorFactory interface
    public:
        AbstractEditor* create();
        bool canOpen(QUrl url);
};

#endif // WIDGETHOLDEREDITORFACTORY_H
