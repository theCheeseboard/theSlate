#-------------------------------------------------
#
# Project created by QtCreator 2017-05-31T12:32:23
#
#-------------------------------------------------

QT       += core gui svg network websockets
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theSlate
TEMPLATE = app

macx {
    QT += macextras
    ICON = icon.icns
    LIBS += -framework CoreFoundation
}

unix:!macx {
    QT += thelib
    TARGET = theslate
}

win32 {
    RC_FILE = icon.rc
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tabbutton.cpp \
    texteditor.cpp \
    aboutwindow.cpp \
    syntaxhighlighter.cpp \
    SourceControl/gitintegration.cpp

HEADERS += \
        mainwindow.h \
    tabbutton.h \
    texteditor.h \
    aboutwindow.h \
    syntaxhighlighter.h \
    SourceControl/gitintegration.h \
    exception.h

FORMS += \
        mainwindow.ui \
    aboutwindow.ui

RESOURCES += \
    icons.qrc \
    files.qrc

TRANSLATIONS += translations/vi_VN.ts

unix:!macx {
    SOURCES += terminalwidget.cpp
    HEADERS += terminalwidget.h

    target.path = /usr/bin

    translations.path = /usr/share/theslate/translations
    translations.files = translations/*

    INSTALLS += target translations
}

macx {
    translations.files = translations/
    translations.path = Contents/translations

    QMAKE_BUNDLE_DATA = translations
}
