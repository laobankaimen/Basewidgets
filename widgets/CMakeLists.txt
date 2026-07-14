# Qt Advanced UI - Widgets library

cmake_minimum_required(VERSION 3.16)

# Options
option(UI_BUILD_WIDGETS "Build Qt Advanced UI widgets library" ON)

# Find Qt
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport X11Extras Svg)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport X11Extras Svg)
    find_package(X11 REQUIRED)
endif()

# Sources
set(UI_PUBLIC_HEADERS
    interface/basewidgets/QtAdvancedUIExport.h
    interface/basewidgets/Styles/acss_globals.h
    interface/basewidgets/Styles/QtAdvancedStylesheet.h
    interface/basewidgets/Core/StyleManagerAdapter.h
    interface/basewidgets/Core/StyledWidget.h
    interface/basewidgets/Core/FramelessDialog.h
    interface/basewidgets/Core/CustomTitleBar.h
    interface/basewidgets/Core/FramelessWidgetWindow.h
    interface/basewidgets/Core/BaseWidgetInterface.h
    interface/basewidgets/Inputs/PushButton.h
    interface/basewidgets/Inputs/LineEdit.h
    interface/basewidgets/Inputs/CheckBox.h
    interface/basewidgets/Inputs/RadioButton.h
    interface/basewidgets/Inputs/ComboBox.h
    interface/basewidgets/Inputs/Slider.h
    interface/basewidgets/Inputs/Label.h
    interface/basewidgets/Inputs/SpinBox.h
    interface/basewidgets/Inputs/ToolButton.h
    interface/basewidgets/Inputs/TextEdit.h
    interface/basewidgets/Inputs/PlainTextEdit.h
    interface/basewidgets/Inputs/DatePicker.h
    interface/basewidgets/Inputs/TimePicker.h
    interface/basewidgets/Inputs/DateTimePicker.h
    interface/basewidgets/Inputs/DateRangePicker.h
    interface/basewidgets/Inputs/ProgressBar.h
    interface/basewidgets/Inputs/ToggleSwitch.h
    interface/basewidgets/Inputs/BusyIndicator.h
    interface/basewidgets/Inputs/MultiSelectComboBox.h
    interface/basewidgets/Containers/GroupBox.h
    interface/basewidgets/Containers/Frame.h
    interface/basewidgets/Containers/TabWidget.h
    interface/basewidgets/Containers/ScrollArea.h
    interface/basewidgets/Containers/MenuBar.h
    interface/basewidgets/Containers/ToolBar.h
    interface/basewidgets/Containers/StatusBar.h
    interface/basewidgets/Containers/ToolBox.h
    interface/basewidgets/Dialogs/DialogUtils.h
    
    interface/basewidgets/Views/TreeWidget.h
    interface/basewidgets/Views/NavigationList.h
    interface/basewidgets/Views/ListView.h
    interface/basewidgets/Views/TreeView.h
    interface/basewidgets/Views/TableView.h
    interface/basewidgets/Views/TreeTable.h
    interface/basewidgets/Views/PagedProxyModel.h
    interface/basewidgets/Views/AdvancedTable.h
    
    interface/basewidgets/Gauges/CircularGauge.h
    interface/basewidgets/Gauges/QcGaugeWidget.h
    interface/basewidgets/Gauges/BasicGauge.h
    interface/basewidgets/Gauges/SpeedGauge.h
    interface/basewidgets/Gauges/CompassGauge.h
    interface/basewidgets/Gauges/AttitudeMeter.h
    
    # Charts
    interface/basewidgets/Charts/QCPBarChart.h
    interface/basewidgets/Charts/QCPLineChart.h
    interface/basewidgets/Charts/QCPWaterfallChart.h
    interface/basewidgets/Charts/QCustomplot.h
    interface/basewidgets/Charts/StyledPlot.h
)

set(UI_SOURCES
    src/Styles/QtAdvancedStylesheet.cpp
    src/Core/StyleManagerAdapter.cpp
    src/Core/StyledWidget.cpp
    src/Core/FramelessDialog.cpp
    src/Core/CustomTitleBar.cpp
    src/Core/FramelessWidgetWindow.cpp
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
    src/Dialogs/DialogUtilsFont.cpp
    src/Dialogs/DialogUtilsInput.cpp
    src/Dialogs/DialogUtilsMessage.cpp
    src/Dialogs/ColorPicker.cpp
    src/Dialogs/FontPicker.cpp
    src/Dialogs/FilePicker.cpp
    
    src/Views/TreeWidget.cpp
    src/Views/NavigationList.cpp
    src/Views/ListView.cpp
    src/Views/TreeView.cpp
    src/Views/TableView.cpp
    src/Views/PagedProxyModel.cpp
    src/Views/AdvancedTable.cpp
    src/Views/TreeTable.cpp
    src/Gauges/CircularGauge.cpp
    src/Gauges/QcGaugeWidget.cpp
    
    # Charts
    src/Charts/QCPBarChart.cpp
    src/Charts/QCPLineChart.cpp
    src/Charts/QCPWaterfallChart.cpp
    src/Charts/QCustomplot.cpp
    src/Charts/StyledPlot.cpp
)

add_library(BaseWidgets SHARED
    ${UI_PUBLIC_HEADERS}
    ${UI_SOURCES}
)
add_library(BaseWidgets::BaseWidgets ALIAS BaseWidgets)

# Export macro
target_compile_definitions(BaseWidgets PRIVATE QTADVANCED_UI_BUILD)

# Include dirs
target_include_directories(BaseWidgets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
        $<INSTALL_INTERFACE:interface>
)

# Link deps
target_link_libraries(BaseWidgets
        PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
        Qt${QT_VERSION_MAJOR}::Svg
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(BaseWidgets
        PUBLIC
            Qt${QT_VERSION_MAJOR}::X11Extras
            ${X11_LIBRARIES}
    )
endif()

# C++ standard
set_target_properties(BaseWidgets PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)

if(MSVC)
    target_compile_options(BaseWidgets PRIVATE /FS)
endif()

set(BASEWIDGETS_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

# 确保 themes.rcc 复制到可执行目录
add_custom_command(TARGET BaseWidgets POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${BASEWIDGETS_RESOURCE_DIR}/themes.rcc
    $<TARGET_FILE_DIR:BaseWidgets>
)

# Install rules
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    install(DIRECTORY assets/
        DESTINATION assets
        COMPONENT ${BASEWIDGETS_COMPONENT_ASSETS}
    )
endif()
install(TARGETS BaseWidgets
    EXPORT BaseWidgetsTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${BASEWIDGETS_COMPONENT_RUNTIME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${BASEWIDGETS_COMPONENT_RUNTIME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
)
install(DIRECTORY interface/basewidgets
    DESTINATION interface
    COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
)
install(EXPORT BaseWidgetsTargets
    FILE BaseWidgetsTargets.cmake
    NAMESPACE BaseWidgets::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BaseWidgets
    COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
)

# 兼容旧发布流程；默认关闭，避免依赖外部发布仓库/脚本。
if(UI_ENABLE_PLUGIN_RELEASE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(plugin_framework_release)
    PF_PUBLISH_FLOW_BEGIN(ENABLE_AUTO_BUILD)
    if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
        PF_PUBLISH_Target("BaseWidgets")
    endif()
endif()
