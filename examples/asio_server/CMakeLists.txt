cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(asio_server VERSION 0.1.0)
include(../../serverdiscovery/CMakeLists.txt)

setup_cpp()
setup_asio()

list(APPEND SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})


