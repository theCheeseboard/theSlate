#include "syntaxhighlighter.h"

#include <QSettings>
#include <QDir>
#include <QScopedPointer>

#ifdef Q_OS_MAC
    extern QString bundlePath;
#endif

QColor parseString(QString s) {
    QStringList parts = s.split(",");
    if (parts.count() != 3) return QColor();

    QColor c(parts.at(0).toInt(), parts.at(1).toInt(), parts.at(2).toInt());
    return c;
}

QColor getSyntaxHighlighterColor(QString color) {
    QString key = color;
    if (!color.contains("/")) {
        color += "/fg";
    }

    QColor background = QApplication::palette("QPlainTextEditor").color(QPalette::Window);
    int avg = (background.blue() + background.green() + background.red()) / 3;

    bool dark = false;
    if (avg < 127) {
        dark = true;
    }

    /*if (color == "comment") {
        return QColor(Qt::gray);
    } else if (color == "class") {
        if (dark) return QColor(Qt::magenta); else return QColor(Qt::darkMagenta);
    } else if (color == "string") {
        if (dark) return QColor(255, 100, 100); else return QColor(Qt::red);
    } else if (color == "function") {
        if (dark) return QColor(Qt::blue); else return QColor(0, 100, 255);
    } else if (color == "keyword") {
        return QColor(255, 150, 0);
    }
    return QColor();*/

    QSettings appSettings;
    QString colPath = appSettings.value("appearance/colDefs", "default").toString();

    QString colFile;
    if (QFile::exists(colPath)) {
        colFile = colPath;
    } else {
        //Load all colour files
        QString targetFileName = dark ? "contemporarydark.ini" : "contemporary.ini";

        QStringList searchPaths;
        #if defined(Q_OS_WIN)
            searchPaths.append(QApplication::applicationDirPath() + "/../../../theSlate/slate/ColorDefinitions/");
            searchPaths.append(QApplication::applicationDirPath() + "/ColorDefinitions/");
        #elif defined(Q_OS_MAC)
            searchPaths.append(bundlePath + "/Contents/Resources/ColorDefinitions");
        #elif (defined Q_OS_UNIX)
            searchPaths.append(QApplication::applicationDirPath() + "/../ColorDefinitions/");
            searchPaths.append("/usr/share/theslate/ColorDefinitions/");
            searchPaths.append(QApplication::applicationDirPath() + "../share/theslate/ColorDefinitions/");
        #endif

        for (QString searchPath : searchPaths) {
            QDir dir(searchPath);
            if (dir.exists()) {
                if (dir.entryList().contains(targetFileName)) {
                    colFile = dir.absoluteFilePath(targetFileName);
                }
            }
        }
    }

    QSettings s(colFile, QSettings::IniFormat);
    if (s.contains(color)) {
        return parseString(s.value(color).toString());
    } else {
        return QColor();
    }
}
