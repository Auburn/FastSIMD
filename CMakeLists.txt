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

option(FASTSIMD_DISPATCH_CLASS "Enable FastSIMD Dispatch Classes" ON)
option(FASTSIMD_EXAMPLES "Build FastSIMD examples" ${FASTSIMD_STANDALONE_PROJECT})
option(FASTSIMD_TESTS "Build FastSIMD tests" ${FASTSIMD_STANDALONE_PROJECT})

add_library(FastSIMD "src/FastSIMD.cpp")

target_include_directories(FastSIMD PUBLIC 
    $<BUILD_INTERFACE:${FastSIMD_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(FastSIMD PRIVATE FASTSIMD_EXPORT)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(FastSIMD PUBLIC FASTSIMD_STATIC_LIB)
endif()

if(FASTSIMD_DISPATCH_CLASS)
    add_subdirectory(DispatchClass)
endif()

if(FASTSIMD_EXAMPLES)
    add_subdirectory(example)
endif()
