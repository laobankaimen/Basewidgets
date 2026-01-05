cmake_minimum_required(VERSION 3.16)

project(QtAdvancedUI LANGUAGES CXX)

# Global C++ standard and Qt auto tools
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Options
option(ACSS_BUILD_STATIC "Build ACSS as static library" OFF)
option(UI_BUILD_EXAMPLES "Build Qt Advanced UI examples" ON)

# MSVC UTF-8 source encoding
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Find Qt (Qt5 or Qt6)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Svg)

# Subdirectories
add_subdirectory(widgets)
if(UI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tools
add_subdirectory(tools)