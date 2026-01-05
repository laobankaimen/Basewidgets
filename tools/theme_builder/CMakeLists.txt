cmake_minimum_required(VERSION 3.16)

project(theme_builder)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

add_executable(theme_builder main.cpp)

target_link_libraries(theme_builder PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    BaseWidgets
)

if(MSVC)
    target_compile_options(theme_builder PRIVATE /FS)
endif()

# 复制 DLL
if(WIN32)
    add_custom_command(TARGET theme_builder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:BaseWidgets>
            $<TARGET_FILE_DIR:theme_builder>
    )
    
    # 复制 styles 目录到构建目录以便测试
    add_custom_command(TARGET theme_builder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/styles
            $<TARGET_FILE_DIR:theme_builder>/styles
    )
endif()
