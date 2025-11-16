# Qt Advanced UI - Widgets library

cmake_minimum_required(VERSION 3.16)

# Options
option(UI_BUILD_WIDGETS "Build Qt Advanced UI widgets library" ON)

if(NOT UI_BUILD_WIDGETS)
    return()
endif()

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

# Sources
set(UI_PUBLIC_HEADERS
    include/QtAdvancedUI/QtAdvancedUIExport.h
    include/QtAdvancedUI/Core/StyleManagerAdapter.h
    include/QtAdvancedUI/Core/StyledWidget.h
    include/QtAdvancedUI/Core/StyledMainWindow.h
    include/QtAdvancedUI/Core/StyledDialog.h
    include/QtAdvancedUI/Core/FramelessDialog.h
    include/QtAdvancedUI/Inputs/PushButton.h
    include/QtAdvancedUI/Inputs/LineEdit.h
    include/QtAdvancedUI/Inputs/CheckBox.h
    include/QtAdvancedUI/Inputs/RadioButton.h
    include/QtAdvancedUI/Inputs/ComboBox.h
    include/QtAdvancedUI/Inputs/Slider.h
    include/QtAdvancedUI/Inputs/Label.h
    include/QtAdvancedUI/Inputs/SpinBox.h
    include/QtAdvancedUI/Inputs/ToolButton.h
    include/QtAdvancedUI/Inputs/TextEdit.h
    include/QtAdvancedUI/Inputs/PlainTextEdit.h
    include/QtAdvancedUI/Inputs/DatePicker.h
    include/QtAdvancedUI/Inputs/TimePicker.h
    include/QtAdvancedUI/Inputs/DateTimePicker.h
    include/QtAdvancedUI/Inputs/DateRangePicker.h
    include/QtAdvancedUI/Inputs/ProgressBar.h
    include/QtAdvancedUI/Inputs/ToggleSwitch.h
    include/QtAdvancedUI/Inputs/BusyIndicator.h
    include/QtAdvancedUI/Inputs/MultiSelectComboBox.h
    include/QtAdvancedUI/Containers/GroupBox.h
    include/QtAdvancedUI/Containers/Frame.h
    include/QtAdvancedUI/Containers/TabWidget.h
    include/QtAdvancedUI/Containers/ScrollArea.h
    include/QtAdvancedUI/Containers/MenuBar.h
    include/QtAdvancedUI/Containers/ToolBar.h
    include/QtAdvancedUI/Containers/StatusBar.h
    include/QtAdvancedUI/Containers/ToolBox.h
    include/QtAdvancedUI/Dialogs/DialogUtils.h
    
    include/QtAdvancedUI/Views/TreeWidget.h
    
    include/QtAdvancedUI/Views/ListView.h
    include/QtAdvancedUI/Views/TreeView.h
    include/QtAdvancedUI/Views/TableView.h
    include/QtAdvancedUI/Views/TreeTable.h
    include/QtAdvancedUI/Views/PagedProxyModel.h
    include/QtAdvancedUI/Views/AdvancedTable.h
    
    include/QtAdvancedUI/Gauges/CircularGauge.h
    
    # Charts
    include/QtAdvancedUI/Charts/QCPBarChart.h
    include/QtAdvancedUI/Charts/QCPLineChart.h
    include/QtAdvancedUI/Charts/QCPWaterfallChart.h
    include/QtAdvancedUI/Charts/QCustomplot.h
    include/QtAdvancedUI/Charts/StyledPlot.h
)

set(UI_SOURCES
    src/Core/StyleManagerAdapter.cpp
    src/Core/StyledWidget.cpp
    src/Core/StyledMainWindow.cpp
    src/Core/StyledDialog.cpp
    src/Core/FramelessDialog.cpp
    src/Inputs/PushButton.cpp
    src/Inputs/LineEdit.cpp
    src/Inputs/CheckBox.cpp
    src/Inputs/RadioButton.cpp
    src/Inputs/ComboBox.cpp
    src/Inputs/Slider.cpp
    src/Inputs/Label.cpp
    src/Inputs/SpinBox.cpp
    src/Inputs/ToolButton.cpp
    src/Inputs/TextEdit.cpp
    src/Inputs/PlainTextEdit.cpp
    src/Inputs/DatePicker.cpp
    src/Inputs/TimePicker.cpp
    src/Inputs/DateTimePicker.cpp
    src/Inputs/DateRangePicker.cpp
    src/Inputs/ProgressBar.cpp
    src/Inputs/ToggleSwitch.cpp
    src/Inputs/BusyIndicator.cpp
    src/Inputs/MultiSelectComboBox.cpp
    src/Containers/GroupBox.cpp
    src/Containers/Frame.cpp
    src/Containers/TabWidget.cpp
    src/Containers/ScrollArea.cpp
    src/Containers/MenuBar.cpp
    src/Containers/ToolBar.cpp
    src/Containers/StatusBar.cpp
    src/Containers/ToolBox.cpp
    src/Dialogs/DialogUtils.cpp
    src/Dialogs/ColorPicker.cpp
    src/Dialogs/FontPicker.cpp
    src/Dialogs/FilePicker.cpp
    
    src/Views/TreeWidget.cpp
    
    src/Views/ListView.cpp
    src/Views/TreeView.cpp
    src/Views/TableView.cpp
    src/Views/PagedProxyModel.cpp
    src/Views/AdvancedTable.cpp
    src/Views/TreeTable.cpp
    src/Gauges/CircularGauge.cpp
    
    # Charts
    src/Charts/QCPBarChart.cpp
    src/Charts/QCPLineChart.cpp
    src/Charts/QCPWaterfallChart.cpp
    src/Charts/QCustomplot.cpp
    src/Charts/StyledPlot.cpp
)

add_library(qtadvancedui_widgets
    ${UI_PUBLIC_HEADERS}
    ${UI_SOURCES}
)

# Export macro
target_compile_definitions(qtadvancedui_widgets PRIVATE QTADVANCEDUI_WIDGETS_BUILD)

# Default styles dir points to project styles
# Consumers可覆盖 UI_STYLES_DIR 定义以替换样式路径
set(UI_DEFAULT_STYLES_DIR "${PROJECT_SOURCE_DIR}/styles")

# Include dirs
target_include_directories(qtadvancedui_widgets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link deps
target_link_libraries(qtadvancedui_widgets
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
        qtadvancedcss
)

# Provide styles dir to library (string literal for compile-time macro)
target_compile_definitions(qtadvancedui_widgets
    PUBLIC
        UI_STYLES_DIR="${UI_DEFAULT_STYLES_DIR}"
)

# C++ standard
set_target_properties(qtadvancedui_widgets PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)

# Install rules
install(TARGETS qtadvancedui_widgets
    EXPORT QtAdvancedUIWidgetsTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/QtAdvancedUI DESTINATION include)