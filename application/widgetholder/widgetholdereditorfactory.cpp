#include "widgetholdereditorfactory.h"

#include "widgetholdereditor.h"

WidgetHolderEditorFactory::WidgetHolderEditorFactory(QObject* parent) :
    AbstractEditorFactory{parent} {
}

AbstractEditor* WidgetHolderEditorFactory::create() {
    return new WidgetHolderEditor();
}

bool WidgetHolderEditorFactory::canOpen(QUrl url) {
    return url.scheme() == "editor";
}
