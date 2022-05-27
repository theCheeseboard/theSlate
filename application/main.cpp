#include <QCommandLineParser>
#include <QDir>
#include <QJsonArray>
#include <QUrl>
#include <tapplication.h>
#include <tsettings.h>
#include <tstylemanager.h>

#include <editormanager.h>
#include <plugins/tpluginmanager.h>
#include <statemanager.h>
#include <theslateplugin.h>

#include "widgetholder/widgetholdereditorfactory.h"

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);

    if (QDir(QStringLiteral("%1/share/theslate/").arg(SYSTEM_PREFIX_DIRECTORY)).exists()) {
        a.setShareDir(QStringLiteral("%1/share/theslate/").arg(SYSTEM_PREFIX_DIRECTORY));
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theslate/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theslate/"));
    }
    a.installTranslators();

    a.setApplicationVersion("2.0");
    a.setGenericName(QApplication::translate("main", "Integrated Development Environment"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2022");
    a.setOrganizationName("theSuite");
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

    a.registerCrashTrap();

#if defined(Q_OS_MAC)
    a.setQuitOnLastWindowClosed(false);
#endif

    tSettings settings;
    QObject::connect(&settings, &tSettings::settingChanged, [=](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

    tPluginManager<TheSlatePlugin> pluginManager;
    pluginManager.load();

    StateManager::editor()->registerFactory("widgetholder", new WidgetHolderEditorFactory());

    MainWindow* w = new MainWindow();
    w->show();

    int retval = a.exec();
    return retval;
}
