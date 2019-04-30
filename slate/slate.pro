#-------------------------------------------------
#
# Project created by QtCreator 2017-05-31T12:32:23
#
#-------------------------------------------------

QT       += core gui svg network concurrent printsupport
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theSlate
TEMPLATE = app

macx {
    QT += macextras
    ICON = icon.icns
    LIBS += -framework CoreFoundation -framework AppKit
    QMAKE_INFO_PLIST = Info.plist
    DEFINES += "THESLATE_END_OF_LINE=0"
}

unix:!macx {
    QT += thelib KSyntaxHighlighting
    TARGET = theslate
    DEFINES += "THESLATE_END_OF_LINE=0"
}

win32 {
    QT += thelib
    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files (x86)/KSyntaxHighlighting/include/KF5/KSyntaxHighlighting"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files (x86)/KSyntaxHighlighting/lib" -lKF5SyntaxHighlighting
    RC_FILE = icon.rc
    DEFINES += "THESLATE_END_OF_LINE=2"
}

macx {
    INCLUDEPATH += "/usr/local/include/the-libs" "/usr/local/include/KF5/KSyntaxHighlighting"
    LIBS += -L/usr/local/lib -lthe-libs -lKF5SyntaxHighlighting
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
    SourceControl/gitintegration.cpp \
    textparts/findreplace.cpp \
    exitsavedialog.cpp \
    textparts/statusbarbutton.cpp \
    textparts/textstatusbar.cpp \
    textparts/topnotification.cpp \
    textparts/mergetool.cpp \
    textparts/printdialog.cpp \
    settingsdialog.cpp \
    picturetabbar.cpp \
    plugins/pluginmanager.cpp \
    managers/recentfilesmanager.cpp \
    managers/updatemanager.cpp \
    managers/syntaxhighlighting.cpp \
    SourceControl/gitwidget.cpp \
    SourceControl/commitsmodel.cpp \
    SourceControl/branchesmodel.cpp \
    SourceControl/GitDialogs/addbranchdialog.cpp \
    SourceControl/GitDialogs/commitdialog.cpp \
    SourceControl/statusmodel.cpp \
    SourceControl/GitDialogs/progressdialog.cpp \
    SourceControl/GitDialogs/authenticationdialog.cpp

HEADERS += \
        mainwindow.h \
    tabbutton.h \
    texteditor.h \
    aboutwindow.h \
    syntaxhighlighter.h \
    SourceControl/gitintegration.h \
    textparts/findreplace.h \
    exitsavedialog.h \
    textparts/statusbarbutton.h \
    textparts/textstatusbar.h \
    textparts/topnotification.h \
    textparts/mergetool.h \
    textparts/printdialog.h \
    settingsdialog.h \
    picturetabbar.h \
    plugins/filebackend.h \
    plugins/pluginmanager.h \
    managers/recentfilesmanager.h \
    managers/updatemanager.h \
    SourceControl/gitwidget.h \
    SourceControl/commitsmodel.h \
    SourceControl/branchesmodel.h \
    SourceControl/GitDialogs/addbranchdialog.h \
    SourceControl/GitDialogs/commitdialog.h \
    SourceControl/statusmodel.h \
    SourceControl/GitDialogs/progressdialog.h \
    SourceControl/GitDialogs/authenticationdialog.h

macx {
    SOURCES += \
        mainwindow-objc.mm \
        main-objc.mm
}

FORMS += \
        mainwindow.ui \
    aboutwindow.ui \
    textparts/findreplace.ui \
    exitsavedialog.ui \
    textparts/textstatusbar.ui \
    textparts/topnotification.ui \
    textparts/mergetool.ui \
    textparts/printdialog.ui \
    settingsdialog.ui \
    picturetabbar.ui \
    SourceControl/gitwidget.ui \
    SourceControl/GitDialogs/addbranchdialog.ui \
    SourceControl/GitDialogs/commitdialog.ui \
    SourceControl/GitDialogs/progressdialog.ui \
    SourceControl/GitDialogs/authenticationdialog.ui

RESOURCES += \
    icons.qrc \
    files.qrc

TRANSLATIONS += translations/vi_VN.ts \
    translations/da_DK.ts \
    translations/es_ES.ts \
    translations/lt_LT.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/ru_RU.ts \
    translations/sv_SE.ts \
    translations/en_AU.ts \
    translations/en_US.ts \
    translations/en_GB.ts \
    translations/en_NZ.ts \
    translations/de_DE.ts \
    translations/id_ID.ts \
    translations/au_AU.ts \
    translations/it_IT.ts \
    translations/nb_NO.ts \
    translations/no_NO.ts \
    translations/ro_RO.ts \
    translations/cy_GB.ts \
    translations/fr_FR.ts

qtPrepareTool(LUPDATE, lupdate)
genlang.commands = "$$LUPDATE -no-obsolete -source-language en_US $$_PRO_FILE_"

qtPrepareTool(LRELEASE, lrelease)
rellang.commands = "$$LRELEASE -removeidentical $$_PRO_FILE_"
QMAKE_EXTRA_TARGETS = genlang rellang
PRE_TARGETDEPS = genlang rellang

# Turn off stripping as this causes the install to fail :(
QMAKE_STRIP = echo

unix:!macx {
    target.path = /usr/bin

    translations.path = /usr/share/theslate/translations
    translations.files = translations/*

    desktop.path = /usr/share/applications
    desktop.files = theslate.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/theslate.svg

    headers.path = /usr/include/theslate
    header.files = plugins/syntaxhighlighting.h plugins/filebackend.h

    cols.files = ColorDefinitions/*
    cols.path = /usr/share/theslate/ColorDefinitions

    INSTALLS += target translations desktop icon headers cols
}

macx {
    translations.files = translations/
    translations.path = Contents/translations

    locversion.files = localisedresources/
    locversion.path = Contents/Resources

    filebackend.files = ../FileBackends/LocalFileBackend/libLocalFileBackend.dylib ../FileBackends/HttpBackend/libHttpBackend.dylib
    filebackend.path = Contents/filebackends/

    cols.files = ColorDefinitions/
    cols.path = Contents/Resources/ColorDefinitions/

    QMAKE_BUNDLE_DATA = translations locversion filebackend cols

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${PWD}/node-appdmg-config.json $${OUT_PWD}/..)
}

DISTFILES += \
    theslate.desktop \
    app-dmg-background.png \
    icon.icns \
    Info.plist \
    ColorDefinitions/themes/Contemporary.theme \
    ColorDefinitions/themes/ContemporaryDark.theme

macx {
    sign.target = sign
    sign.commands = codesign -s theSuite $$OUT_PWD/theSlate.app/

    QMAKE_EXTRA_TARGETS += sign
}
