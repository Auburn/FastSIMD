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
    endif()
endif()

option(FASTSIMD_DISPATCH_CLASS "Enable FastSIMD Dispatch Classes" ON)
option(FASTSIMD_EXAMPLES "Build FastSIMD examples" ${FASTSIMD_STANDALONE_PROJECT})
option(FASTSIMD_TESTS "Build FastSIMD tests" ${FASTSIMD_STANDALONE_PROJECT})

include(cmake/ArchDetect.cmake)

target_architecture(FASTSIMD_ARCH_DETECT FASTSIMD_ARCHVER_DETECT)

add_library(FastSIMD OBJECT "src/FastSIMD.cpp")
target_compile_definitions(FastSIMD PRIVATE FASTSIMD_EXPORT)

if(BUILD_SHARED_LIBS)
    set_property(TARGET FastSIMD PROPERTY POSITION_INDEPENDENT_CODE ON)
else()
    target_compile_definitions(FastSIMD PUBLIC FASTSIMD_STATIC_LIB)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_options(FastSIMD PUBLIC "-msimd128")
endif()

target_include_directories(FastSIMD PUBLIC 
    $<BUILD_INTERFACE:${FastSIMD_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(FASTSIMD_DISPATCH_CLASS)
    add_subdirectory(dispatch)
endif()

if(FASTSIMD_TESTS)
    add_subdirectory(tests)
endif()

if(FASTSIMD_EXAMPLES)
    add_subdirectory(examples)
endif()
