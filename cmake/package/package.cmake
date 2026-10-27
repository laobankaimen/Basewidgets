include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(BASEWIDGETS_PACKAGE_NAME "basewidgets" CACHE STRING "Package name used by CPack")
set(BASEWIDGETS_PACKAGE_VENDOR "zhoutao" CACHE STRING "Package vendor")
set(BASEWIDGETS_PACKAGE_DESCRIPTION "UI开发基础控件库" CACHE STRING "Package description")
set(BASEWIDGETS_PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/package" CACHE PATH "Package output directory")
set(BASEWIDGETS_INSTALL_TREE_DIR "${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install" CACHE PATH "Installed package tree output directory")
set(BASEWIDGETS_PACKAGE_FILE_BASENAME "${BASEWIDGETS_PACKAGE_NAME}" CACHE STRING "Base name used in generated package file names")
set(BASEWIDGETS_PACKAGE_FILE_VERSION "${PROJECT_VERSION}" CACHE STRING "Version segment used in generated package file names")
set(BASEWIDGETS_PACKAGE_PLATFORM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "Platform segment used in generated package file names")
set(BASEWIDGETS_CPACK_GENERATORS "" CACHE STRING "Optional semicolon-separated list of CPack generators")

set(BASEWIDGETS_COMPONENT_RUNTIME "Runtime")
set(BASEWIDGETS_COMPONENT_DEVELOPMENT "Development")
set(BASEWIDGETS_COMPONENT_ASSETS "Assets")

option(BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX "Allow installing to the system-wide CMAKE_INSTALL_PREFIX (e.g. /usr/local). OFF by default to avoid polluting the system." OFF)
mark_as_advanced(BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX)

set(_BASEWIDGETS_SYSTEM_PREFIXES
    "/usr/local"
    "/usr"
    "C:/Program Files/${PROJECT_NAME}"
    "C:/Program Files (x86)/${PROJECT_NAME}"
    "C:/Program Files"
    "C:/Program Files (x86)"
)
set(_BASEWIDGETS_IS_SYSTEM_PREFIX OFF)
foreach(_PREFIX IN LISTS _BASEWIDGETS_SYSTEM_PREFIXES)
    if("${CMAKE_INSTALL_PREFIX}" STREQUAL "${_PREFIX}")
        set(_BASEWIDGETS_IS_SYSTEM_PREFIX ON)
        break()
    endif()
endforeach()

if(NOT BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX AND _BASEWIDGETS_IS_SYSTEM_PREFIX)
    message(STATUS "BaseWidgets: CMAKE_INSTALL_PREFIX (${CMAKE_INSTALL_PREFIX}) is a system prefix but BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX=OFF, using '${BASEWIDGETS_INSTALL_TREE_DIR}' instead.")
    set(CMAKE_INSTALL_PREFIX "${BASEWIDGETS_INSTALL_TREE_DIR}" CACHE PATH "Install prefix" FORCE)
endif()

install(FILES "${CMAKE_SOURCE_DIR}/README.md"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
)

# Keep the installed README's relative docs/image links usable offline.
install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}/docs"
    COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
    FILES_MATCHING PATTERN "*.md" PATTERN "*.png"
)

set(BASEWIDGETS_CMAKE_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/BaseWidgets")

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/package/BaseWidgetsConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/BaseWidgetsConfig.cmake"
    INSTALL_DESTINATION "${BASEWIDGETS_CMAKE_CONFIG_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/BaseWidgetsConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/BaseWidgetsConfig.cmake"
    "${CMAKE_BINARY_DIR}/BaseWidgetsConfigVersion.cmake"
    DESTINATION "${BASEWIDGETS_CMAKE_CONFIG_DIR}"
    COMPONENT ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
)

if(CMAKE_CONFIGURATION_TYPES)
    set(_basewidgets_install_config_args --config $<CONFIG>)
else()
    set(_basewidgets_install_config_args)
    if(CMAKE_BUILD_TYPE)
        list(APPEND _basewidgets_install_config_args --config "${CMAKE_BUILD_TYPE}")
    endif()
endif()

add_custom_target(package_dir
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
            ${_basewidgets_install_config_args}
            --prefix "${BASEWIDGETS_INSTALL_TREE_DIR}"
    COMMENT "Installing package tree to ${BASEWIDGETS_INSTALL_TREE_DIR}"
    VERBATIM
)

add_custom_target(package_runtime_dir
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
            ${_basewidgets_install_config_args}
            --component "${BASEWIDGETS_COMPONENT_RUNTIME}"
            --prefix "${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-runtime"
    COMMENT "Installing runtime component to ${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-runtime"
    VERBATIM
)

add_custom_target(package_development_dir
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
            ${_basewidgets_install_config_args}
            --component "${BASEWIDGETS_COMPONENT_DEVELOPMENT}"
            --prefix "${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-development"
    COMMENT "Installing development component to ${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-development"
    VERBATIM
)

add_custom_target(package_assets_dir
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
            ${_basewidgets_install_config_args}
            --component "${BASEWIDGETS_COMPONENT_ASSETS}"
            --prefix "${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-assets"
    COMMENT "Installing assets component to ${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/install-assets"
    VERBATIM
)

if(UI_ENABLE_CPACK)
    if(BASEWIDGETS_CPACK_GENERATORS)
        set(CPACK_GENERATOR "${BASEWIDGETS_CPACK_GENERATORS}")
    elseif(WIN32)
        set(CPACK_GENERATOR "ZIP")
    else()
        set(CPACK_GENERATOR "TGZ;ZIP")
    endif()

    set(CPACK_PACKAGE_NAME "${BASEWIDGETS_PACKAGE_NAME}")
    set(CPACK_PACKAGE_VENDOR "${BASEWIDGETS_PACKAGE_VENDOR}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${BASEWIDGETS_PACKAGE_DESCRIPTION}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DIRECTORY "${BASEWIDGETS_PACKAGE_OUTPUT_DIR}")
    set(CPACK_PACKAGE_FILE_NAME
        "${BASEWIDGETS_PACKAGE_FILE_BASENAME}-${BASEWIDGETS_PACKAGE_FILE_VERSION}-${BASEWIDGETS_PACKAGE_PLATFORM_NAME}")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_COMPONENTS_ALL
        ${BASEWIDGETS_COMPONENT_RUNTIME}
        ${BASEWIDGETS_COMPONENT_DEVELOPMENT}
        ${BASEWIDGETS_COMPONENT_ASSETS}
    )
    set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "BaseWidgets Runtime")
    set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Shared libraries and runtime binaries.")
    set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "BaseWidgets Development")
    set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Headers, CMake package config, export targets, and documentation.")
    set(CPACK_COMPONENT_ASSETS_DISPLAY_NAME "BaseWidgets Assets")
    set(CPACK_COMPONENT_ASSETS_DESCRIPTION "Theme resources such as themes.rcc.")
    set(CPACK_SOURCE_IGNORE_FILES
        "/build/;/cmake-build-.*/;/.git/;/.idea/;/.vscode/;${BASEWIDGETS_PACKAGE_OUTPUT_DIR}/")

    include(CPack)
endif()
