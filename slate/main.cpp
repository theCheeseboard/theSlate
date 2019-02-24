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
#include <Repository>
#include <Theme>

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
    QString bundlePath;

    extern void setupMacObjC();
#endif

QLinkedList<QString> clipboardHistory;
PluginManager* plugins;
RecentFilesManager* recentFiles;
UpdateManager* updateManager;
KSyntaxHighlighting::Repository* highlightRepo;
KSyntaxHighlighting::Theme highlightTheme;

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

    //a.registerCrashTrap();

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
#elif defined(Q_OS_WIN)
    //Check if dark palette is needed
    QSettings darkDetection("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    int AppsUseLightTheme = darkDetection.value("AppsUseLightTheme", 1).toInt();

    if (AppsUseLightTheme == 0) {
        //Set up the dark palette
        a.setStyle(QStyleFactory::create("contemporary"));

        //Get the accent colour
        QSettings accentDetection("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
        QColor accentCol(QRgb(accentDetection.value("ColorizationColor", 0xC4003296).toInt() & 0x00FFFFFF));

        QPalette pal = a.palette();

        pal.setColor(QPalette::Button, accentCol);
        pal.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        pal.setColor(QPalette::Highlight, accentCol.lighter(125));
        pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        pal.setColor(QPalette::Disabled, QPalette::Button, accentCol.darker(200));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));

        pal.setColor(QPalette::Window, QColor(40, 40, 40));
        pal.setColor(QPalette::Base, QColor(40, 40, 40));
        pal.setColor(QPalette::AlternateBase, QColor(60, 60, 60));
        pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
        pal.setColor(QPalette::Text, QColor(255, 255, 255));
        pal.setColor(QPalette::ToolTipText, QColor(255, 255, 255));

        pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));

        a.setPalette(pal);
        a.setPalette(pal, "QDockWidget");
        a.setPalette(pal, "QToolBar");
    }
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

    highlightRepo = new KSyntaxHighlighting::Repository();
    #ifdef Q_OS_MAC
        highlightRepo->addCustomSearchPath(bundlePath + "/Contents/Resources/ColorDefinitions");
    #elif defined(Q_OS_WIN)
        highlightRepo->addCustomSearchPath(QApplication::applicationDirPath() + "/../../../theSlate/slate/ColorDefinitions/");
        highlightRepo->addCustomSearchPath(QApplication::applicationDirPath() + "/ColorDefinitions/");
    #else
        highlightRepo->addCustomSearchPath(QDir::cleanPath(QApplication::applicationDirPath() + "/../../theSlate/slate/ColorDefinitions/"));
        highlightRepo->addCustomSearchPath("/usr/share/theslate/ColorDefinitions/");
        highlightRepo->addCustomSearchPath(QDir::cleanPath(QApplication::applicationDirPath() + "../share/theslate/ColorDefinitions/"));
    #endif

    QColor background = QApplication::palette("QPlainTextEditor").color(QPalette::Window);
    int avg = (background.blue() + background.green() + background.red()) / 3;
    bool dark = false;
    if (avg < 127) {
        dark = true;
    }
    if (dark) {
        highlightTheme = highlightRepo->theme("Contemporary Dark");
    } else {
        highlightTheme = highlightRepo->theme("Contemporary");
    }

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
