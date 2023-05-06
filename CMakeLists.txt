cmake_minimum_required(VERSION 3.10)

project(accel-socket CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(ACCEL_BUILD_TESTS ON)

if(NOT DEFINED ACCEL_MODULES_FOLDER)
    set(ACCEL_MODULES_FOLDER "${CMAKE_SOURCE_DIR}/external")
endif()

set(ADDITIONAL_LIBRARIES accel-macros)
set(ADDITIONAL_INCLUDES "")
set(ADDITIONAL_SOURCES "")

if(WIN32)
    list(APPEND ADDITIONAL_LIBRARIES ws2_32)
endif()

include(cmake/FindModule.cmake)
add_accel_module(accel-macros)

add_library(accel-socket INTERFACE)
target_include_directories(accel-socket INTERFACE "include/")
target_link_libraries(accel-socket INTERFACE ${ADDITIONAL_LIBRARIES})

if(ACCEL_BUILD_TESTS)
    add_subdirectory(tests)
endif()