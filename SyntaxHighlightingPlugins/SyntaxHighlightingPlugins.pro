#-------------------------------------------------
#
# Project created by QtCreator 2018-08-04T22:26:07
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = SyntaxHighlightingPlugins
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

SOURCES += \
        defaultsyntaxhighlighting.cpp \
    highlighters/cppsyntaxhighlighter.cpp \
    highlighters/xmlsyntaxhighlighter.cpp \
    highlighters/jssyntaxhighlighter.cpp \
    highlighters/jsonsyntaxhighlighter.cpp

HEADERS += \
        defaultsyntaxhighlighting.h \
    highlighters/cppsyntaxhighlighter.h \
    highlighters/xmlsyntaxhighlighter.h \
    highlighters/jssyntaxhighlighter.h \
    highlighters/jsonsyntaxhighlighter.h

DISTFILES += SyntaxHighlightingPlugins.json 

unix:!macx {
    target.path = /usr/share/theslate/syntaxhighlighting

    INSTALLS += target
}

macx {
    sign.target = sign
    QMAKE_EXTRA_TARGETS += sign
}
