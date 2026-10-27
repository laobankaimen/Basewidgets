cmake_minimum_required(VERSION 3.16)

project(widgets_demo)

# Qt dependencies are selected by the root CMakeLists.txt.

# `widgets_demo` 用于仓库内联编调试，直接链接源码树中的 `BaseWidgets`。
# 安装包/发布包的外部消费方式请参考 `examples/package_consumer`。
add_executable(widgets_demo
    main.cpp
    pages/DialogsPage.cpp
    pages/InputsPage.cpp
    pages/GaugesPage.cpp
    pages/AcousticPage.cpp
    pages/DirectionAnalysisPage.cpp
    pages/DataManagementPage.cpp
    pages/PagedTablePage.cpp
    pages/TextPage.cpp
    pages/ViewsPage.cpp
)

target_link_libraries(widgets_demo
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        BaseWidgets
)

if(MSVC)
    target_compile_options(widgets_demo PRIVATE /FS)
endif()

# Windows needs the DLL beside the executable. Themes are embedded in that DLL;
# no themes.rcc copy or runtime source-tree lookup is required.
if(WIN32)
    add_custom_command(TARGET widgets_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:BaseWidgets> $<TARGET_FILE_DIR:widgets_demo>)
endif()

set_target_properties(widgets_demo PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)

option(UI_DEMO_CHECKS "Enable widgets_demo --verify (requires Qt Test)" OFF)
if(UI_DEMO_CHECKS)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
    target_sources(widgets_demo PRIVATE DemoChecks.cpp)
    target_link_libraries(widgets_demo PRIVATE Qt${QT_VERSION_MAJOR}::Test)
    target_compile_definitions(widgets_demo PRIVATE UI_DEMO_CHECKS BASEWIDGETS_SOURCE_DIR="${CMAKE_SOURCE_DIR}" BASEWIDGETS_GENERATED_RCC="${BASEWIDGETS_GENERATED_RCC}")
endif()
