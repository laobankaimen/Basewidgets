cmake_minimum_required(VERSION 3.16)

project(widgets_demo)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

add_executable(widgets_demo
    main.cpp
    pages/PagedTablePage.cpp
)

target_link_libraries(widgets_demo
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        QtAdvancedUI
)

if(MSVC)
    target_compile_options(widgets_demo PRIVATE /FS)
endif()

# 确保 QtAdvancedUI.dll 复制到可执行目录
add_custom_command(TARGET widgets_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:QtAdvancedUI>
            $<TARGET_FILE_DIR:widgets_demo>
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE_DIR:QtAdvancedUI>/themes.rcc"
            $<TARGET_FILE_DIR:widgets_demo>
)

set_target_properties(widgets_demo PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)