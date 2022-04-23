# CMakeList.txt : CMake project for FastSIMD
cmake_minimum_required(VERSION 3.7.1)

project(FastSIMD VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# Build DLL
#set(BUILD_SHARED_LIBS ON) 

include(cmake/ClassSIMD.cmake)

add_library(fastsimd_headers INTERFACE)
target_include_directories(fastsimd_headers INTERFACE "include")

add_library(fastsimd "src/FastSIMD.cpp")
target_link_libraries(fastsimd PUBLIC fastsimd_headers)

add_subdirectory(example)
