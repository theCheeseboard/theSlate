#ifndef TOPNOTIFICATION_H
#define TOPNOTIFICATION_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
    class TopNotification;
}

class TopNotification : public QWidget
{
        Q_OBJECT

    public:
        explicit TopNotification(QWidget *parent = nullptr);
        ~TopNotification();

    public slots:
        void setTitle(QString title);
        void setText(QString text);
        void addButton(QPushButton* button);

    private:
        Ui::TopNotification *ui;
};

#endif // TOPNOTIFICATION_H
