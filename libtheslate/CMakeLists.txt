cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)

set(SOURCES
    statemanager.cpp

    editormanager.cpp
    editors/abstracteditor/abstracteditor.cpp
    editors/abstracteditor/abstracteditorfactory.cpp
    editors/abstracteditor/abstracteditorcolorscheme.cpp
    editors/texteditor/texteditor.cpp
    editors/texteditor/texteditorfactory.cpp
    editors/texteditor/textcaret.cpp
    editors/texteditor/commands/carettextcommand.cpp
)

set(HEADERS
    libtheslate_global.h

    statemanager.h

    editormanager.h
    editors/abstracteditor/abstracteditor.h
    editors/abstracteditor/abstracteditorfactory.h
    editors/abstracteditor/abstracteditorcolorscheme.h
)

set(PRIVATE_HEADERS
    editors/texteditor/texteditor_p.h
    editors/texteditor/texteditor.h
    editors/texteditor/textcaret.h
    editors/texteditor/texteditorfactory.h
    editors/texteditor/commands/carettextcommand.h
)

add_library(libtheslate SHARED ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
cntp_init(libtheslate 17)
set_target_properties(libtheslate PROPERTIES
        OUTPUT_NAME theslate
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.theslate
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libtheslate Qt6::Widgets libcontemporary)
target_compile_definitions(libtheslate PRIVATE LIBTHESLATE_LIBRARY)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libtheslate.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/libtheslate.framework/Headers)
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libtheslate)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libtheslate)
    set(INCLUDE_INSTALL_DIR ${HEADER_INSTALL_DIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

configure_package_config_file(libtheslateConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libtheslateConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtheslateConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libtheslateTargets
        FILE libtheslateTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libtheslate
        EXPORT libtheslateTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()
