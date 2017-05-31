#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include "texteditor.h"

class TextEditor;

class TabButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TabButton(QWidget *parent = 0);
    explicit TabButton(TextEditor* editor, QWidget *parent = 0);

    void setActive(bool active);
signals:

public slots:

private slots:
    void updateTitle(QString title);
    void updateIcon(QIcon icon);

private:
    TextEditor* editor;
    bool active;

    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};

#endif // TABBUTTON_H
