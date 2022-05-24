cmake_minimum_required(VERSION 3.4.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
        plugin.cpp)

set(HEADERS
        plugin.h)

add_library(plugin-cmake SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-cmake PUBLIC ../../libtheslate/)

cntp_init_plugin(theslate plugin-cmake 17 macintegration)
cntp_translate(plugin-cmake)

set_target_properties(plugin-cmake PROPERTIES
        OUTPUT_NAME macIntegrationPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-cmake Qt6::Widgets Qt6::Multimedia libcontemporary)
