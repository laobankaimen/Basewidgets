# Exporter example

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

add_executable(exporter
    exporter.cpp
)

target_link_libraries(exporter PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    qtadvancedcss
)

# Match qmake define for styles path
# Use project root styles directory
target_compile_definitions(exporter PRIVATE
    STYLES_DIR=${PROJECT_SOURCE_DIR}/styles
    QT_DEPRECATED_WARNINGS
)

# Ensure qtadvancedcss.dll is beside the executable after build
add_custom_command(TARGET exporter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qtadvancedcss>
            $<TARGET_FILE_DIR:exporter>
)