cmake_minimum_required(VERSION 3.21.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
        cmakebuildengine.cpp
        plugin.cpp
        cmakerunconfiguration.cpp

        buildjobs/cmakeconfigurejob.cpp
        buildjobs/cmakebuildjob.cpp

        fileapiclient/replyindex.cpp
        fileapiclient/codemodelobject.cpp
        fileapiclient/targetobject.cpp
        fileapiclient/directoryobject.cpp
)

set(HEADERS
        plugin.h
        cmakebuildengine.h
        cmakerunconfiguration.h

        buildjobs/cmakeconfigurejob.h
        buildjobs/cmakebuildjob.h

        fileapiclient/replyindex.h
        fileapiclient/codemodelobject.h
        fileapiclient/targetobject.h
        fileapiclient/directoryobject.h
)

add_library(plugin-cmake SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-cmake PUBLIC ../../libtheslate/)

cntp_init_plugin(theslate plugin-cmake 20 plugin-cmake)
cntp_translate(plugin-cmake)

set_target_properties(plugin-cmake PROPERTIES
        OUTPUT_NAME cmakePlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-cmake Qt6::Widgets Qt6::Multimedia libcontemporary libtheslate)
