cmake_minimum_required(VERSION 3.16)
project(OpenRGBAdalightPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets SerialPort)

set(OPENRGB_INCLUDE_DIR "" CACHE PATH "Pfad zum OpenRGB Quellcode Verzeichnis")

include_directories(
    ${OPENRGB_INCLUDE_DIR}
    ${OPENRGB_INCLUDE_DIR}/RGBController
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
    OpenRGBAdalightPlugin.cpp
    AdalightDevice.cpp
    AdalightWidget.cpp
)

set(HEADERS
    OpenRGBAdalightPlugin.h
    AdalightDevice.h
    AdalightWidget.h
)

add_library(OpenRGBAdalightPlugin SHARED ${SOURCES} ${HEADERS})

target_link_libraries(OpenRGBAdalightPlugin
    Qt5::Core
    Qt5::Widgets
    Qt5::SerialPort
)

# Output DLL direkt in den OpenRGB Plugins Ordner
set_target_properties(OpenRGBAdalightPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$ENV{APPDATA}/OpenRGB/plugins"
)
