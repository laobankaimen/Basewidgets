# Qt Advanced UI - Widgets library

# Find Qt (full component set; root CMakeLists only finds the base subset)
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
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/QtAdvancedUIExport.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Styles/acss_globals.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Styles/QtAdvancedStylesheet.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/StyleManagerAdapter.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/StyledWidget.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/FramelessDialog.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/CustomTitleBar.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/FramelessWidgetWindow.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Core/BaseWidgetInterface.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/PushButton.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/LineEdit.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/CheckBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/RadioButton.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/ComboBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/Slider.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/Label.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/SpinBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/ToolButton.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/TextEdit.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/PlainTextEdit.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/DatePicker.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/TimePicker.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/DateTimePicker.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/DateRangePicker.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/ProgressBar.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/ToggleSwitch.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/BusyIndicator.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Inputs/MultiSelectComboBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/GroupBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/Frame.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/TabWidget.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/ScrollArea.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/MenuBar.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/ToolBar.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/StatusBar.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Containers/ToolBox.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Dialogs/DialogUtils.h

    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/TreeWidget.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/NavigationList.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/ListView.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/TreeView.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/TableView.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/TreeTable.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/PagedProxyModel.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Views/AdvancedTable.h

    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/CircularGauge.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/QcGaugeWidget.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/BasicGauge.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/SpeedGauge.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/CompassGauge.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Gauges/AttitudeMeter.h

    # Charts
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Charts/QCPBarChart.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Charts/QCPLineChart.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Charts/QCPWaterfallChart.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Charts/QCustomplot.h
    ${CMAKE_SOURCE_DIR}/interface/basewidgets/Charts/StyledPlot.h
)

set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Styles/QtAdvancedStylesheet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/StyleManagerAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/StyledWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/FramelessDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/CustomTitleBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/FramelessWidgetWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/PushButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/LineEdit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/CheckBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/RadioButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/ComboBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/Slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/Label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/SpinBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/ToolButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/TextEdit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/PlainTextEdit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/DatePicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/TimePicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/DateTimePicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/DateRangePicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/ProgressBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/ToggleSwitch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/BusyIndicator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs/MultiSelectComboBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/GroupBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/Frame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/TabWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/ScrollArea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/MenuBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/ToolBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/StatusBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Containers/ToolBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/DialogUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/DialogUtilsFont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/DialogUtilsInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/DialogUtilsMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/ColorPicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/FontPicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/FilePicker.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/TreeWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/NavigationList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/ListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/TreeView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/TableView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/PagedProxyModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/AdvancedTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/TreeTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gauges/CircularGauge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gauges/QcGaugeWidget.cpp

    # Charts
    ${CMAKE_CURRENT_SOURCE_DIR}/Charts/QCPBarChart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Charts/QCPLineChart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Charts/QCPWaterfallChart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Charts/QCustomplot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Charts/StyledPlot.cpp
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
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/interface>
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

set(BASEWIDGETS_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

# 确保 themes.rcc 复制到可执行目录
add_custom_command(TARGET BaseWidgets POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${BASEWIDGETS_RESOURCE_DIR}/themes.rcc
    $<TARGET_FILE_DIR:BaseWidgets>
)

# Install rules
if(NOT UI_ENABLE_PLUGIN_RELEASE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
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
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/interface/basewidgets
        DESTINATION interface
        COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
    )
    install(EXPORT BaseWidgetsTargets
        FILE BaseWidgetsTargets.cmake
        NAMESPACE BaseWidgets::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BaseWidgets
        COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
    )
endif()
