# Full features GUI example

# Find Qt including QuickWidgets and Qml
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets QuickWidgets Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets QuickWidgets Qml)

# WIN32 on Windows to avoid console window
add_executable(full_features WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    full_features.qrc
)

# Link against Qt and the library
target_link_libraries(full_features PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Qml
    qtadvancedcss
)

# Match qmake define for styles path
# Styles directory define
# Use project root styles directory
target_compile_definitions(full_features PRIVATE
    STYLES_DIR=${PROJECT_SOURCE_DIR}/styles
    QT_DEPRECATED_WARNINGS
)

# Ensure qtadvancedcss.dll is beside the executable after build
add_custom_command(TARGET full_features POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qtadvancedcss>
            $<TARGET_FILE_DIR:full_features>
)