# Theme sources are the only editable inputs. Generated artifacts live in the build tree.
set(BASEWIDGETS_THEME_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/styles")
set(BASEWIDGETS_GENERATED_RCC "${CMAKE_CURRENT_BINARY_DIR}/generated/assets/themes.rcc")
set(BASEWIDGETS_GENERATED_RESOURCE_CPP "${CMAKE_CURRENT_BINARY_DIR}/generated/themes_resource.cpp")

# Reuse the theme parser/SVG generator without linking the UI library: linking BaseWidgets
# here would create the cycle generator -> BaseWidgets -> generated resources -> generator.
add_executable(basewidgets_resource_builder EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/resources/theme_builder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Styles/QtAdvancedStylesheet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/basewidgets/Styles/QtAdvancedStylesheet.h")
target_include_directories(basewidgets_resource_builder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/interface")
target_compile_definitions(basewidgets_resource_builder PRIVATE QTADVANCED_UI_BUILD)
target_link_libraries(basewidgets_resource_builder PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)
set_target_properties(basewidgets_resource_builder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/internal")

# Cross builds cannot run a target-platform executable on the build host.
# Supply an equivalent host generator built from this source with the same resource format.
set(BASEWIDGETS_HOST_RESOURCE_BUILDER "" CACHE FILEPATH "Host resource generator for cross compilation")
if(CMAKE_CROSSCOMPILING)
    if(NOT BASEWIDGETS_HOST_RESOURCE_BUILDER)
        message(FATAL_ERROR "Cross compiling requires BASEWIDGETS_HOST_RESOURCE_BUILDER (host executable).")
    endif()
    set(_basewidgets_builder "${BASEWIDGETS_HOST_RESOURCE_BUILDER}")
else()
    set(_basewidgets_builder "$<TARGET_FILE:basewidgets_resource_builder>")
endif()

# CONFIGURE_DEPENDS also picks up newly added fonts/icons/XML files. Changed content reruns
# the custom command; an unchanged second build does no resource work.
file(GLOB_RECURSE _basewidgets_theme_inputs CONFIGURE_DEPENDS
    "${BASEWIDGETS_THEME_SOURCE_DIR}/*")
add_custom_command(
    OUTPUT "${BASEWIDGETS_GENERATED_RCC}" "${BASEWIDGETS_GENERATED_RESOURCE_CPP}"
    COMMAND ${CMAKE_COMMAND} -E env "QT_QPA_PLATFORM=offscreen"
        "${_basewidgets_builder}"
        --styles-dir "${BASEWIDGETS_THEME_SOURCE_DIR}"
        --output "${BASEWIDGETS_GENERATED_RCC}"
        --cpp-output "${BASEWIDGETS_GENERATED_RESOURCE_CPP}"
        --rcc "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::rcc>"
    DEPENDS "${_basewidgets_builder}" Qt${QT_VERSION_MAJOR}::rcc ${_basewidgets_theme_inputs}
    COMMENT "Generating and embedding BaseWidgets themes (JSON/XML/fonts/SVG)"
    VERBATIM)
add_custom_target(basewidgets_resources DEPENDS
    "${BASEWIDGETS_GENERATED_RCC}" "${BASEWIDGETS_GENERATED_RESOURCE_CPP}")
