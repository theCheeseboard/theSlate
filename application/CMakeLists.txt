cmake_minimum_required(VERSION 3.21.0)

project(theslate VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)

set(SOURCES
        main.cpp
        mainwindow.cpp mainwindow.ui

        pages/abstractpage/abstractpage.cpp
        pages/editorpage/editorpage.cpp pages/editorpage/editorpage.ui
        unsavedchangespopover.cpp unsavedchangespopover.ui
)

set(HEADERS
        mainwindow.h
        pages/abstractpage/abstractpage.h
        pages/editorpage/editorpage.h
        unsavedchangespopover.h
)

# Find Apple lproj files
#set(LPROJ_SOURCES)
#file(GLOB LPROJ_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/translations/apple-lproj/*.lproj)
#FOREACH(DIR ${LPROJ_DIRS})
#    cmake_path(GET DIR FILENAME DIR_NAME)
#    file(GLOB LPROJ_FILES ${DIR}/*.strings)
#    FOREACH(FILE ${LPROJ_FILES})
#        list(APPEND LPROJ_SOURCES ${FILE})
#        set_source_files_properties(${FILE} PROPERTIES
#                MACOSX_PACKAGE_LOCATION Resources/${DIR_NAME})
#    ENDFOREACH()
#ENDFOREACH()
#set(SOURCES ${SOURCES} ${LPROJ_SOURCES})

add_executable(theslate ${SOURCES} ${HEADERS})

set_target_properties(theslate PROPERTIES
        MACOSX_BUNDLE TRUE
#        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.theslate
        WIN32_EXECUTABLE TRUE
        CNTP_DATA_SUBDIR theslate)

cntp_init(theslate 17)
cntp_translate(theslate)
cntp_target_name(theslate "theSlate")
cntp_app_icon(theslate
    BASEICON icons/baseicon.svg
    BASEICON_MAC icons/baseicon-mac.svg)
cntp_target_desktop_file(theslate
    DESKTOP_FILE com.vicr123.theslate.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.theslate_blueprint.desktop)
cntp_defaults_file(theslate
    DEFAULTS_FILE theslate.conf)

target_link_libraries(theslate Qt6::Widgets libcontemporary libtheslate)
target_include_directories(theslate PUBLIC ../libtheslate/)

install(TARGETS theslate
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
