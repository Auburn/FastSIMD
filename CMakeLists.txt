# CMakeList.txt : CMake project for FastSIMD
cmake_minimum_required(VERSION 3.7.1)

project(FastSIMD VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)

# determine whether this is a standalone project or included by other projects
if (NOT DEFINED FASTSIMD_STANDALONE_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(FASTSIMD_STANDALONE_PROJECT ON)
    else()
        set(FASTSIMD_STANDALONE_PROJECT OFF)
    endif ()
endif()

option(FASTSIMD_EXAMPLES "Build FastSIMD examples" ${FASTSIMD_STANDALONE_PROJECT})
option(FASTSIMD_TESTS "Build FastSIMD tests" ${FASTSIMD_STANDALONE_PROJECT})

# Build DLL
#set(BUILD_SHARED_LIBS ON) 

include(cmake/ClassSIMD.cmake)

add_library(fastsimd_headers INTERFACE)
target_include_directories(fastsimd_headers INTERFACE "include")

add_library(fastsimd "src/FastSIMD.cpp")
target_link_libraries(fastsimd PUBLIC fastsimd_headers)

if(FASTSIMD_EXAMPLES)
    add_subdirectory(example)
endif()
