# CMakeList.txt : CMake project for FastSIMD
cmake_minimum_required(VERSION 3.7.1)

project(FastSIMD VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# Build DLL
#set(BUILD_SHARED_LIBS ON) 

include(cmake/ClassSIMD.cmake)

add_library(fastsimd INTERFACE)
target_include_directories(fastsimd INTERFACE "include")

add_subdirectory(example)
