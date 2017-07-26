#include "terminalwidget.h"

TerminalWidget::TerminalWidget(QString workDir, QWidget *parent) : QTermWidget(0, parent)
{
    this->setScrollBarPosition(QTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme("Linux");
    this->setHistorySize(-1);

    QFont font;
    font.setFamily("Hack");
    font.setPointSize(10);
    this->setTerminalFont(font);

    QStringList environment;
    environment.append("TERM=xterm");

    this->setEnvironment(environment);

    if (workDir != "") {
        this->setWorkingDirectory(workDir);
    }

    this->update();
    this->startShellProgram();
}

void TerminalWidget::runCommand(QString command) {
    this->sendText(command + "\n");
}
