#-------------------------------------------------
#
# Project created by QtCreator 2017-05-31T12:32:23
#
#-------------------------------------------------

QT       += core gui svg network concurrent printsupport webenginewidgets
CONFIG   += c++14
SHARE_APP_NAME=theslate

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    QT += thelib KSyntaxHighlighting
    TARGET = theslate
    DEFINES += "THESLATE_END_OF_LINE=0"

    target.path = /usr/bin

    desktop.path = /usr/share/applications
    desktop.files = theslate.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/theslate.svg

    headers.path = /usr/include/theslate
    header.files = plugins/syntaxhighlighting.h plugins/filebackend.h

    cols.files = ColorDefinitions/*
    cols.path = /usr/share/theslate/ColorDefinitions

    INSTALLS += target desktop icon headers cols
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    QT += thelib
    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files (x86)/KSyntaxHighlighting/include/KF5/KSyntaxHighlighting"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files (x86)/KSyntaxHighlighting/lib" -lKF5SyntaxHighlighting
    RC_FILE = icon.rc
    DEFINES += "THESLATE_END_OF_LINE=2"
    TARGET = theSlate
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    QT += macextras
    LIBS += -framework CoreFoundation -framework AppKit
    QMAKE_INFO_PLIST = Info.plist
    DEFINES += "THESLATE_END_OF_LINE=0"

    blueprint {
        TARGET = "theSlate Blueprint"
        ICON = icon-bp.icns
    } else {
        TARGET = "theSlate"
        ICON = icon.icns

        sign.commands = codesign -s theSuite $$OUT_PWD/theSlate.app/
        sign.target = sign
        QMAKE_EXTRA_TARGETS += sign
    }

    INCLUDEPATH += "/usr/local/include/the-libs" "/usr/local/include/KF5/KSyntaxHighlighting"
    LIBS += -L/usr/local/lib -lthe-libs -lKF5SyntaxHighlighting

    locversion.files = localisedresources/
    locversion.path = Contents/Resources

    filebackend.files = ../FileBackends/LocalFileBackend/libLocalFileBackend.dylib ../FileBackends/HttpBackend/libHttpBackend.dylib
    filebackend.path = Contents/filebackends/

    auxpane.files = ../AuxiliaryPanes/HtmlPreview/libHtmlPreview.dylib ../AuxiliaryPanes/MarkdownPreview/libMdPreview.dylib
    auxpane.path = Contents/auxiliarypanes/

    cols.files = ColorDefinitions/
    cols.path = Contents/Resources/ColorDefinitions/

    QMAKE_BUNDLE_DATA += locversion filebackend auxpane cols

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${PWD}/node-appdmg-config*.json $${OUT_PWD}/..)
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
    aboutwindow.cpp \
    SyntaxHighlighting/syntaxhighlighter.cpp \
    SourceControl/gitintegration.cpp \
    textparts/findreplace.cpp \
    exitsavedialog.cpp \
    textparts/selectlistdialog.cpp \
    textparts/statusbarbutton.cpp \
    textparts/texteditor.cpp \
    textparts/texteditorblockdata.cpp \
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
    SourceControl/GitDialogs/authenticationdialog.cpp \
    textwidget.cpp

HEADERS += \
        mainwindow.h \
    plugins/auxiliarypane.h \
    tabbutton.h \
    aboutwindow.h \
    SyntaxHighlighting/syntaxhighlighter.h \
    SourceControl/gitintegration.h \
    textparts/findreplace.h \
    exitsavedialog.h \
    textparts/selectlistdialog.h \
    textparts/statusbarbutton.h \
    textparts/texteditor.h \
    textparts/texteditorblockdata.h \
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
    SourceControl/GitDialogs/authenticationdialog.h \
    textwidget.h

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
    textparts/selectlistdialog.ui \
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
    SourceControl/GitDialogs/authenticationdialog.ui \
    textwidget.ui

RESOURCES += \
    icons.qrc \
    files.qrc

# Turn off stripping as this causes the install to fail :(
QMAKE_STRIP = echo

DISTFILES += \
    theslate.desktop \
    app-dmg-background.png \
    icon.icns \
    Info.plist \
    ColorDefinitions/themes/Contemporary.theme \
    ColorDefinitions/themes/ContemporaryDark.theme
