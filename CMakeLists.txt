cmake_minimum_required(VERSION 3.10)

project(accel-socket CXX)

option(BUILD_TESTS "Build the accompanying tests." ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/accel/*.hpp")

add_library(accel-socket-static STATIC ${SRC_FILES})
target_include_directories(accel-socket-static PUBLIC "include/")
target_link_libraries(accel-socket-static PUBLIC ${ADDITIONAL_LIBRARIES})

add_library(accel-socket SHARED ${SRC_FILES})
target_compile_definitions(accel-socket PUBLIC -DSHARED_BUILD PRIVATE -DACCEL_EXPORTS)
target_include_directories(accel-socket PUBLIC "include/")
target_link_libraries(accel-socket PUBLIC ${ADDITIONAL_LIBRARIES})

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()