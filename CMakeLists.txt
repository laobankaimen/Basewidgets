cmake_minimum_required(VERSION 3.16)

project(QtAdvancedUI VERSION 1.0.1 LANGUAGES CXX)

# Global C++ standard and Qt auto tools
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Options
option(ACSS_BUILD_STATIC "Build ACSS as static library" OFF)
option(UI_BUILD_EXAMPLES "Build Qt Advanced UI examples" ON)
option(UI_ENABLE_PLUGIN_RELEASE "Enable legacy plugin release workflow" OFF)
option(UI_ENABLE_CPACK "Enable CPack packaging targets" ON)

# MSVC UTF-8 source encoding
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Select Qt once for the library, examples, tools, and exported package.
# Linux window integration uses QX11Info from Qt5 X11Extras. A cached QT_DIR
# may still point to Qt6, so do not use the versionless lookup on Linux.
set(_basewidgets_qt_components Core Gui Widgets PrintSupport Svg)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Qt5 5.11 REQUIRED COMPONENTS ${_basewidgets_qt_components} X11Extras)
    set(QT_VERSION_MAJOR 5)
    find_package(X11 REQUIRED)
else()
    find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS ${_basewidgets_qt_components})
    if(QT_VERSION_MAJOR EQUAL 5)
        find_package(Qt5 5.11 REQUIRED COMPONENTS ${_basewidgets_qt_components})
    else()
        find_package(Qt6 REQUIRED COMPONENTS ${_basewidgets_qt_components})
    endif()
endif()
message(STATUS "BaseWidgets: Using Qt${QT_VERSION_MAJOR} (${Qt${QT_VERSION_MAJOR}_DIR})")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if(UI_ENABLE_PLUGIN_RELEASE)
    message(STATUS "BaseWidgets: Using LEGACY plugin release workflow (UI_ENABLE_PLUGIN_RELEASE=ON)")
    include(../com.plugin.releases/plugin_framework_release.cmake)
    PF_PUBLISH_FLOW_BEGIN(ENABLE_AUTO_BUILD)
else()
    message(STATUS "BaseWidgets: Using CMake/CPack release workflow (UI_ENABLE_PLUGIN_RELEASE=OFF)")
    include(package/package)
endif()

# Unified output directories (baselib-style): runtime -> bin/<Config>, shared objects -> lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 内部资源生成器不依赖 BaseWidgets，先建立工具→资源→UI 库的单向依赖。
include(resources/resources)

# Subdirectories
add_subdirectory(src)
if(UI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


if(UI_ENABLE_PLUGIN_RELEASE)
    PF_PUBLISH_TARGET("BaseWidgets")
endif()

if(UI_BUILD_EXAMPLES AND UI_DEMO_CHECKS)
    enable_testing()
    add_test(NAME widgets_behavior COMMAND widgets_demo --verify)
    set_tests_properties(widgets_behavior PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
