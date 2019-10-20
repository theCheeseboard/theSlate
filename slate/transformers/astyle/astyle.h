#ifndef ASTYLE_H
#define ASTYLE_H

#include <QObject>

struct AStylePrivate;
class QFile;
class AStyle : public QObject
{
        Q_OBJECT
    public:
        explicit AStyle(QObject *parent = nullptr);
        ~AStyle();

        static bool isAStyleAvailable();

        QString settingsContents();
        void setSettingsContents(QString settingsContents);

        QByteArray doAStyle(QByteArray input, QString filename, QString* error);

    signals:

    public slots:
        void resetSettings();
        void saveSettings();

    private:
        AStylePrivate* d;

        QString configFilePath();
        void loadSettings();
        void saveSettings(QFile* configFile, bool replaceVars);
};

#endif // AStyle_H
