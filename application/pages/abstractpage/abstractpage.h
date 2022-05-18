#ifndef ABSTRACTPAGE_H
#define ABSTRACTPAGE_H

#include <QWidget>

class tWindowTabberButton;
class AbstractPage : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractPage(QWidget* parent = nullptr);

        virtual tWindowTabberButton* tabButton() = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;

    signals:
};

#endif // ABSTRACTPAGE_H
