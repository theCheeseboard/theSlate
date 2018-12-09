#include "mainwindow.h"
#include <tapplication.h>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopServices>
#include <QCommandLineParser>
#include <QClipboard>
#include "aboutwindow.h"
#include "settingsdialog.h"
#include "plugins/pluginmanager.h"
#include "managers/recentfilesmanager.h"
#include "managers/updatemanager.h"

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
    QString bundlePath;

    extern void setupMacObjC();
#endif

QLinkedList<QString> clipboardHistory;
PluginManager* plugins;
RecentFilesManager* recentFiles;
UpdateManager* updateManager;

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

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

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
    setupMacObjC();
#endif

    updateManager = new UpdateManager();

    QCommandLineParser parser;
    parser.setApplicationDescription(a.translate("AboutWindow", "Text Editor"));
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {
            {"i", "stdin"}, a.translate("main", "Read from standard input")
        }
    });
    parser.addPositionalArgument(a.translate("main", "files"), a.translate("main", "Files to open"), a.translate("main", "[files...]"));
    parser.process(a);

    //Set up plugins
    plugins = new PluginManager();
    recentFiles = new RecentFilesManager();

    MainWindow* w = new MainWindow();

    if (parser.isSet("i")) {
        //We want to read from stdin
        QTextStream s(stdin, QIODevice::ReadOnly);

        QStringList buf;
        QString currentLine;
        do {
            currentLine = s.readLine();
            buf.append(currentLine);
        } while (currentLine != "");

        w->newTab(buf.join("\n").toUtf8());
    }

    for (QString arg : parser.positionalArguments()) {
        w->newTab(arg);
    }

    QObject::connect(&a, &tApplication::openFile, [=](QString file) {
        if (MainWindow::openWindows.count() == 0) {
            MainWindow* w = new MainWindow();
            w->newTab(file);
            w->show();
        } else {
            MainWindow::openWindows.first()->newTab(file);
        }
    });
    QObject::connect(a.clipboard(), &QClipboard::dataChanged, [=] {
        const QMimeData* d = QApplication::clipboard()->mimeData();
        if (d->hasText()) {
            clipboardHistory.prepend(d->text());
            if (clipboardHistory.size() > 10) clipboardHistory.takeLast();
        }
    });

    w->show();

    return a.exec();
}
