#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include "textparts/texteditor.h"

class TextEditor;

class TabButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TabButton(QWidget *parent = nullptr);
    explicit TabButton(TextEditor* editor, QWidget *parent = nullptr);

    void setActive(bool active);
signals:

public slots:

private slots:
    void updateTitle(QString title);
    void updateIcon(QIcon icon);

private:
    TextEditor* editor;
    bool active = false;

    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
};

#endif // TABBUTTON_H
