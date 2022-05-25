cmake_minimum_required(VERSION 3.4.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
        cmakebuildengine.cpp
        plugin.cpp
        cmakerunconfiguration.cpp

        buildjobs/cmakeconfigurejob.cpp
        buildjobs/cmakebuildjob.cpp
)

set(HEADERS
        plugin.h
        cmakebuildengine.h
        cmakerunconfiguration.h

        buildjobs/cmakeconfigurejob.h
        buildjobs/cmakebuildjob.h
)

add_library(plugin-cmake SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-cmake PUBLIC ../../libtheslate/)

cntp_init_plugin(theslate plugin-cmake 17 plugin-cmake)
cntp_translate(plugin-cmake)

set_target_properties(plugin-cmake PROPERTIES
        OUTPUT_NAME cmakePlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-cmake Qt6::Widgets Qt6::Multimedia libcontemporary libtheslate)
