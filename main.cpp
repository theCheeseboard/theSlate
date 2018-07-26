#include "mainwindow.h"
#include <tapplication.h>
#include <QTranslator>
#include <QLibraryInfo>

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
#endif

int main(int argc, char *argv[])
{
    tApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theSlate");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator localTranslator;
#ifdef Q_OS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);

    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
                                           kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath,
                                           CFStringGetSystemEncoding());

    localTranslator.load(QLocale::system().name(), QString::fromLocal8Bit(pathPtr) + "/Contents/translations/");

    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif

#ifdef Q_OS_LINUX
    localTranslator.load(QLocale::system().name(), "/usr/share/theslate/translations");
#endif

    a.installTranslator(&localTranslator);

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
