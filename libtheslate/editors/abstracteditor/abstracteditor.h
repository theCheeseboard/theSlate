#ifndef ABSTRACTEDITOR_H
#define ABSTRACTEDITOR_H

#include <QWidget>

class AbstractEditorColorScheme;
struct AbstractEditorPrivate;
class AbstractEditor : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractEditor(QWidget* parent = nullptr);
        ~AbstractEditor();

        AbstractEditorColorScheme* colorScheme();
        void setColorScheme(QString scheme);

    signals:

    private:
        AbstractEditorPrivate* d;
};

#endif // ABSTRACTEDITOR_H
