cmake_minimum_required(VERSION 3.16)

project(basewidgets_package_consumer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(BaseWidgets CONFIG REQUIRED)

add_executable(package_consumer_demo
    main.cpp
)

target_link_libraries(package_consumer_demo PRIVATE BaseWidgets::BaseWidgets)
