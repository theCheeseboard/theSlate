#include "mainwindow.h"
#include <tapplication.h>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopServices>
#include "aboutwindow.h"
#include "settingsdialog.h"

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
    QString bundlePath;
#endif

void setupMacMenubar() {
    //Set up macOS menu bar when no window is open
    QMenuBar* menubar = new QMenuBar();

    QMenu* fileMenu = new QMenu(tApplication::translate("MainWindow", "File"));
    fileMenu->addAction(tApplication::translate("MainWindow", "New"), [=] {
        MainWindow* w = new MainWindow();
        w->show();
    }, QKeySequence(Qt::ControlModifier | Qt::Key_N));
    fileMenu->addAction(tApplication::translate("MainWindow", "New Window"), [=] {
        MainWindow* w = new MainWindow();
        w->show();
    });
    fileMenu->addSeparator();
    fileMenu->addAction(tApplication::translate("MainWindow", "Settings"), [=] {
        SettingsDialog d;
        d.exec();
    })->setMenuRole(QAction::PreferencesRole);
    fileMenu->addSeparator();
    fileMenu->addAction(tApplication::translate("MainWindow", "Exit"), [=] {
        tApplication::quit();
    })->setMenuRole(QAction::QuitRole);
    menubar->addMenu(fileMenu);

    QMenu* helpMenu = new QMenu(tApplication::translate("MainWindow", "Help"));
    helpMenu->addAction(tApplication::translate("MainWindow", "File Bug"), [=] {
        QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theslate/issues"));
    });
    helpMenu->addAction(tApplication::translate("MainWindow", "Sources"), [=] {
        QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theslate"));
    });
    helpMenu->addSeparator();
    helpMenu->addAction(tApplication::translate("MainWindow", "About"), [=] {
        AboutWindow aboutWindow;
        aboutWindow.exec();
    })->setMenuRole(QAction::AboutRole);

    menubar->addMenu(helpMenu);
}

int main(int argc, char *argv[])
{
    tApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theSlate");

    qDebug() << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Services");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide %1");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide Others");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Show All");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Preferences...");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "About %1");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Quit %1");

    QTranslator localTranslator;
#ifdef Q_OS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
    a.setQuitOnLastWindowClosed(false);

    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());

    bundlePath = QString::fromLocal8Bit(pathPtr);
    localTranslator.load(QLocale::system().name(), bundlePath + "/Contents/translations/");

    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif

#ifdef Q_OS_LINUX
    localTranslator.load(QLocale::system().name(), "/usr/share/theslate/translations");
#endif

#ifdef Q_OS_WIN
    localTranslator.load(QLocale::system().name(), a.applicationDirPath() + "\\translations");
#endif

    a.installTranslator(&localTranslator);

#ifdef Q_OS_MAC
    setupMacMenubar();
#endif

    QStringList args = a.arguments();
    args.takeFirst();

    MainWindow* w = new MainWindow();

    for (QString arg : args) {
        if (arg.startsWith("--")) {

        } else {
            w->newTab(arg);
        }
    }

    QObject::connect(&a, &tApplication::openFile, [=](QString file) {
        w->newTab(file);
    });

    w->show();

    return a.exec();
}
