cmake_minimum_required(VERSION 3.6)

project(accel-socket LANGUAGES CXX)

set(CXX_STANDARD 11)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/accel-socket/*.hpp")

add_library(accel-socket SHARED )