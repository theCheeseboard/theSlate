#ifndef ABSTRACTLEFTPANE_H
#define ABSTRACTLEFTPANE_H

#include <QWidget>

class tWindowTabberButton;
class AbstractLeftPane : public QWidget {
        Q_OBJECT
    public:
        explicit AbstractLeftPane(QWidget* parent = nullptr);

        virtual tWindowTabberButton* tabButton() = 0;

    signals:
};

#endif // ABSTRACTLEFTPANE_H
