#ifndef STATUSBARBUTTON_H
#define STATUSBARBUTTON_H

#include <QToolButton>

class StatusBarButton : public QToolButton
{
        Q_OBJECT
    public:
        explicit StatusBarButton(QWidget *parent = nullptr);

        QSize sizeHint() const;
    signals:

    public slots:

    private:
        void paintEvent(QPaintEvent* event);
};

#endif // STATUSBARBUTTON_H
