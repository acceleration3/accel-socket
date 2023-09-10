cmake_minimum_required(VERSION 3.10)

project(accel-sockets CXX)

set(ACCEL_BUILD_TESTS ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED ACCEL_MODULES_FOLDER)
    set(ACCEL_MODULES_FOLDER "${CMAKE_SOURCE_DIR}/external")
endif()

include(cmake/FindModule.cmake)
add_accel_module(accel-macros)
add_accel_module(accel-endianess)
add_accel_module(accel-utf8)
add_accel_module(accel-result)

set(ADDITIONAL_LIBRARIES accel-macros accel-endianess accel-utf8 accel-result)
set(ADDITIONAL_INCLUDES "")
set(ADDITIONAL_SOURCES "")

if(WIN32)
    list(APPEND ADDITIONAL_LIBRARIES ws2_32)
endif()

add_library(accel-sockets INTERFACE)
target_include_directories(accel-sockets INTERFACE "include/")
target_link_libraries(accel-sockets INTERFACE ${ADDITIONAL_LIBRARIES})

if(ACCEL_BUILD_TESTS)
    add_subdirectory(tests)
endif()