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
