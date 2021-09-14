cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(servicediscovery VERSION 0.1.0)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/5.12.6/msvc2017_64/lib/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
enable_language(CXX)

add_subdirectory("serverdiscovery")
add_subdirectory("examples/qt_server")
add_subdirectory("examples/qt_client")
add_subdirectory("examples/asio_server")
add_subdirectory("examples/asio_client")
add_subdirectory("tests/test_message")
