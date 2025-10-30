cmake_minimum_required(VERSION 3.16)

project(widgets_demo)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

add_executable(widgets_demo
    main.cpp
    pages/FormPage.cpp
    pages/ContainersPage.cpp
    pages/ViewsPage.cpp
    pages/DialogsPage.cpp
    pages/ToolBoxPage.cpp
    pages/DashboardPage.cpp
    pages/PagedTablePage.cpp
    pages/ChartsPage.cpp
)

target_link_libraries(widgets_demo
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        qtadvancedui_widgets
        qtadvancedcss
)

# 确保 qtadvancedcss.dll 复制到可执行目录
add_custom_command(TARGET widgets_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qtadvancedcss>
            $<TARGET_FILE_DIR:widgets_demo>
)

set_target_properties(widgets_demo PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)