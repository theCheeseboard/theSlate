cmake_minimum_required(VERSION 3.21.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_package(tdotnetbridge REQUIRED)

set(SOURCES
        plugin.cpp
)

set(HEADERS
        plugin.h
)

add_library(plugin-dotnet SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-dotnet PUBLIC ../../libtheslate/)

cntp_init_plugin(theslate plugin-dotnet 20 plugin-dotnet)
cntp_translate(plugin-dotnet)

tdotnet_add_dotnet_project(plugin-theslate-dotnet-dotnetintegration
    PROJECT DotNetPluginDotNet/DotNetIntegration/DotNetIntegration.csproj)

set_target_properties(plugin-dotnet PROPERTIES
        OUTPUT_NAME dotnetPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-dotnet Qt6::Widgets Qt6::Multimedia libcontemporary libtheslate tdotnetbridge plugin-theslate-dotnet-dotnetintegration)
