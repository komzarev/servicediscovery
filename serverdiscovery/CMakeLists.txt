cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

set (COMMON_PATH ${CMAKE_CURRENT_LIST_DIR})
include_directories(${COMMON_PATH})

macro(setup_ssdp_qt)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Network REQUIRED)
    set(CMAKE_AUTOMOC ON )

    set( SOURCES
        ${COMMON_PATH}/ssdp_qt.cpp
    )

    set( HEADERS
        ${COMMON_PATH}/ssdp_qt.hpp
    )

endmacro()

macro(setup_cpp)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    # set(CMAKE_CXX_EXTENSIONS OFF)

    add_definitions(-D_WIN32_WINNT=0x0A00)
    
    if(MSVC)
        string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
    endif()

endmacro()

macro(setup_asio)

    include_directories(${CMAKE_SOURCE_DIR}/dep/optional/include)
    include_directories(${CMAKE_SOURCE_DIR}/dep/boost-ho/include)

    message(${CMAKE_SOURCE_DIR}/dep/optional/include)
    add_definitions( -DBOOST_ASIO_STANDALONE  -DBOOST_ASIO_SEPARATE_COMPILATION)

    set( SOURCES
        ${COMMON_PATH}/ssdp_asio.cpp
        ${COMMON_PATH}/asio_cache.cpp
    )

    set( HEADERS
        ${COMMON_PATH}/ssdp_asio.hpp
    )

endmacro()

