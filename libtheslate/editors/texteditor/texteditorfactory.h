#ifndef TEXTEDITORFACTORY_H
#define TEXTEDITORFACTORY_H

#include "../abstracteditor/abstracteditorfactory.h"

class TextEditorFactory : public AbstractEditorFactory {
        Q_OBJECT
    public:
        explicit TextEditorFactory(QObject* parent = nullptr);

        AbstractEditor* create();
        bool canOpen(QUrl url);

    signals:
};

#endif // TEXTEDITORFACTORY_H
