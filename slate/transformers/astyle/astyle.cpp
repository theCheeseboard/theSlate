#include "astyle.h"

#include <the-libs_global.h>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>
#include <QProcess>

struct AStylePrivate {
    QString settingsContents;
    QSettings settings;
};

AStyle::AStyle(QObject *parent) : QObject(parent)
{
    d = new AStylePrivate();

    if (!QFile::exists(this->configFilePath())) {
        //Copy over default settings
        resetSettings();
    } else {
        loadSettings();
    }
}

AStyle::~AStyle()
{
    delete d;
}

bool AStyle::isAStyleAvailable()
{
    if (theLibsGlobal::searchInPath("astyle").isEmpty()) {
        return false;
    } else {
        return true;
    }
}

QString AStyle::settingsContents()
{
    return d->settingsContents;
}

void AStyle::setSettingsContents(QString settingsContents)
{
    d->settingsContents = settingsContents;
}

QByteArray AStyle::doAStyle(QByteArray input, QString filename, QString* error)
{
    if (!isAStyleAvailable()) {
        *error = "Artistic Style is not installed";
        return "";
    }

    //Prepare the settings file
    QTemporaryFile file("AStyleXXXXXX");
    file.open();
    this->saveSettings(&file, true);
    file.close();

    QString suffix = QFileInfo(filename).suffix();
    QString modeEntry;
    if (suffix == "cs") {
        modeEntry = QStringLiteral("--mode=cs");
    } else if (suffix == "java") {
        modeEntry = QStringLiteral("--mode=java");
    } else {
        modeEntry = QStringLiteral("--mode=c");
    }

    QStringList bits = {
        QStringLiteral("\"%1\"").arg(theLibsGlobal::searchInPath("astyle").first()),
        QStringLiteral("--options=\"%1\"").arg(file.fileName()),
        modeEntry
    };

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QProcess proc;
    proc.setProcessEnvironment(env);
    proc.start(bits.join(" "));

    //Write in the file to be beautified
    proc.write(input);
    proc.closeWriteChannel();

    //Wait for the output
    proc.waitForFinished();

    if (proc.exitCode() == 0) {
        return proc.readAllStandardOutput();
    } else {
        *error = tr("Artistic Style returned exit code %1:").arg(proc.exitCode()).append("\n").append(proc.readAllStandardError());
        return "";
    }
}

void AStyle::resetSettings()
{
    //Copy the default config over
    QFile::copy(":/transformers/astyle/defaultconfig", this->configFilePath());

    //Set permissions properly (not sure why it screws up on macOS)
    QFile configFile(this->configFilePath());
    configFile.setPermissions(QFile::WriteUser | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);

    loadSettings();
}

void AStyle::saveSettings()
{
    QFile configFile(this->configFilePath());
    configFile.open(QFile::WriteOnly);
    this->saveSettings(&configFile, false);
    configFile.close();
}

QString AStyle::configFilePath()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!configDir.exists()) QDir::root().mkpath(configDir.absolutePath());

    return configDir.absoluteFilePath("astyle");
}

void AStyle::loadSettings() {
    //Special settings:
    //%[INDENT_SIZE]: Indentation size configured in Settings
    //%[INDENT_TABS]: Whether to use tabs as configured in Settings

    QFile configFile(this->configFilePath());
    configFile.open(QFile::ReadOnly);
    d->settingsContents = configFile.readAll();
    configFile.close();
}

void AStyle::saveSettings(QFile* configFile, bool replaceVars)
{
    QString output = d->settingsContents;
    if (replaceVars) {
        output.replace("%[INDENT_SIZE]", QString::number(d->settings.value("behaviour/tabSpaceNumber", 4).toInt()));
        output.replace("%[INDENT_TABS]", d->settings.value("behaviour/tabSpaces", true).toBool() ? "spaces" : "tab");
    }
    configFile->write(output.toUtf8());
}
