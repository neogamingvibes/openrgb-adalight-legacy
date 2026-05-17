cmake_minimum_required(VERSION 3.16)
project(OpenRGBAdalightPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt5 path
set(CMAKE_PREFIX_PATH "C:/Users/the_r/Qt/5.15.2/msvc2019_64" CACHE PATH "Qt5 installation path")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# OpenRGB source path
set(OPENRGB_INCLUDE_DIR "D:/Arbeit/openrgb-adalight-legacy/OpenRGB-master" CACHE PATH "Path to the OpenRGB source directory")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENRGB_INCLUDE_DIR}
    ${OPENRGB_INCLUDE_DIR}/RGBController
    ${OPENRGB_INCLUDE_DIR}/i2c_smbus
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
)

# Output DLL directly into the OpenRGB plugins folder
set_target_properties(OpenRGBAdalightPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$ENV{APPDATA}/OpenRGB/plugins"
)
