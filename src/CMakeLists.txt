# Library: qtadvancedcss

set(ACSS_HEADERS
    acss_globals.h
    QmlStyleUrlInterceptor.h
    QtAdvancedStylesheet.h
)

set(ACSS_SOURCES
    QmlStyleUrlInterceptor.cpp
    QtAdvancedStylesheet.cpp
)

if(ACSS_BUILD_STATIC)
    add_library(qtadvancedcss STATIC ${ACSS_HEADERS} ${ACSS_SOURCES})
    target_compile_definitions(qtadvancedcss PUBLIC ACSS_STATIC)
else()
    add_library(qtadvancedcss SHARED ${ACSS_HEADERS} ${ACSS_SOURCES})
    target_compile_definitions(qtadvancedcss PRIVATE ACSS_SHARED_EXPORT)
endif()

# C++ standard for this target (redundant but explicit)
target_compile_features(qtadvancedcss PUBLIC cxx_std_14)

# Include directories
# Public includes allow consumers (examples) to include headers
target_include_directories(qtadvancedcss PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Qt linkage
target_link_libraries(qtadvancedcss
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Svg
)

# Common defines and MSVC encoding flag
target_compile_definitions(qtadvancedcss PUBLIC QT_DEPRECATED_WARNINGS)
target_compile_options(qtadvancedcss PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Install rules (standard CMake install prefix)
install(TARGETS qtadvancedcss
        EXPORT QtAdvancedStylesheetsTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${ACSS_HEADERS}
        DESTINATION include)