#-------------------------------------------------
#
# Project created by QtCreator 2019-05-25T20:25:07
#
#-------------------------------------------------

QT       += core gui widgets webenginewidgets

unix:!macx {
    QT += thelib
}

win32 {
    QT += thelib
    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs
}

macx {
    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs
}


TARGET = HtmlPreview
TEMPLATE = lib
CONFIG += plugin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


DISTFILES += HtmlBackend.json

unix:!macx {
    target.path = /usr/share/theslate/auxiliarypanes

    INSTALLS += target
}

HEADERS += \
    htmlpreviewpane.h \
    plugin.h \
    ../../slate/plugins/auxiliarypane.h \ #Also moc the backend header
    webpage.h

SOURCES += \
    htmlpreviewpane.cpp \
    plugin.cpp \
    webpage.cpp

FORMS += \
    htmlpreviewpane.ui
