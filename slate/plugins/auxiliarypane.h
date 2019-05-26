#ifndef AUXILIARYPANE_H
#define AUXILIARYPANE_H

#include <QTextCursor>
#include <QWidget>
#include <functional>
#define THESLATE_AUXILIARYPANE_IID "org.thesuite.theSlate.AuxiliaryPane"

class AuxiliaryPane : public QWidget {
    Q_OBJECT
    public:
        AuxiliaryPane(QWidget* parent = nullptr) : QWidget(parent) {}

        virtual void parseFile(QUrl fileName, QString fileData) = 0;
        virtual void cursorChanged(QTextCursor cursor) = 0;

    signals:
        void updateFile(QByteArray fileContents);
};

struct AuxiliaryPaneCapabilities {
    QString name;
    std::function<AuxiliaryPane*()> makePane;
};

class AuxiliaryPanePlugin {
    public:
        virtual ~AuxiliaryPanePlugin() {}

        virtual QList<AuxiliaryPaneCapabilities> getPanes() = 0;
        virtual QList<AuxiliaryPaneCapabilities> getPanesForUrl(QUrl url) = 0;
};
Q_DECLARE_INTERFACE(AuxiliaryPanePlugin, THESLATE_AUXILIARYPANE_IID)

#endif // AUXILIARYPANE_H
