cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(test_message VERSION 0.1.0)
include(../../serverdiscovery/CMakeLists.txt)
enable_testing()

set(CMAKE_AUTOMOC ON )
setup_cpp()
find_package(Qt5Test REQUIRED)

list(APPEND SOURCES
    tst_parser.cpp
)
include_directories(${CMAKE_SOURCE_DIR}/dep/optional/include)
add_executable(${PROJECT_NAME} ${SOURCES})

add_test(NAME Parser COMMAND Parser)
target_link_libraries(test_message Qt5::Test)


