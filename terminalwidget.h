#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <qtermwidget5/qtermwidget.h>
#include <QObject>

class TerminalWidget : public QTermWidget
{
    Q_OBJECT
public:
    explicit TerminalWidget(QString workDir = "", QWidget *parent = 0);

signals:

public slots:
    void runCommand(QString command);

};

#endif // TERMINALWIDGET_H
