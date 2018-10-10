#ifndef PICTURETABBAR_H
#define PICTURETABBAR_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

namespace Ui {
    class PictureTabBar;
}

class PictureTabBar : public QWidget
{
        Q_OBJECT

    public:
        explicit PictureTabBar(QWidget *parent = nullptr);
        ~PictureTabBar();

        QSize sizeHint() const;

    private:
        Ui::PictureTabBar *ui;

        void paintEvent(QPaintEvent* paintEvent);
};

#endif // PICTURETABBAR_H
