cmake_minimum_required(VERSION 3.21.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
        plugin.cpp
        clanglogscanner.cpp
)

set(HEADERS
        plugin.h
        clanglogscanner.h
)

add_library(plugin-clang SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-clang PUBLIC ../../libtheslate/)

cntp_init_plugin(theslate plugin-clang 20 plugin-clang)
cntp_translate(plugin-clang)

set_target_properties(plugin-clang PROPERTIES
        OUTPUT_NAME clangPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-clang Qt6::Widgets Qt6::Multimedia libcontemporary libtheslate)
