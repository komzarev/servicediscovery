cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(qt_client VERSION 0.1.0)
include(../../serverdiscovery/CMakeLists.txt)

setup_cpp()
setup_ssdp_qt()

list(APPEND SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Qt5::Network)

