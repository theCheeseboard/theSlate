#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QObject>
#include <QMessageBox>

class MessageBox : public QMessageBox
{
        Q_OBJECT
    public:
        explicit MessageBox(QWidget *parent = nullptr);

    signals:

    public slots:
        void setWindowTitle(QString windowTitle);
        void setText(QString text);
};

#endif // MESSAGEBOX_H
