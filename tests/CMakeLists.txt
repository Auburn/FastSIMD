
set(FASTSIMD_TEST_FEATURE_SETS SCALAR SSE2 SSE41 AVX2 AVX512 NEON AARCH64 WASM)

# Each register size is its own pair of dispatch libraries, test.inl instantiates FASTSIMD_TEST_REGISTER_BYTES
# All sizes in one translation unit needs ~4.5GB per unit in a GCC Debug build, too much for 4 parallel jobs on a CI runner
# Keep in sync with kTestRegisterBytes in test.h
set(simd_test_libraries "")
foreach(register_bytes 16 32 64 128)
  fastsimd_create_dispatch_library(simd_test_${register_bytes} SOURCES "test.inl" FEATURE_SETS ${FASTSIMD_TEST_FEATURE_SETS})
  fastsimd_create_dispatch_library(simd_test_relaxed_${register_bytes} RELAXED SOURCES "test.inl" FEATURE_SETS ${FASTSIMD_TEST_FEATURE_SETS})

  foreach(simd_test_library simd_test_${register_bytes} simd_test_relaxed_${register_bytes})
    target_compile_definitions(${simd_test_library} PRIVATE FASTSIMD_TEST_REGISTER_BYTES=${register_bytes})

    # Every test is too many sections for an unoptimised COFF object file
    if(MSVC)
      target_compile_options(${simd_test_library} PRIVATE /bigobj)
    elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      # GNU as (MinGW) has the same limit, clang's integrated assembler switches to big objects itself
      target_compile_options(${simd_test_library} PRIVATE -Wa,-mbig-obj)
    endif()

    list(APPEND simd_test_libraries ${simd_test_library})
  endforeach()
endforeach()

# "test" is a reserved target name when CTest is enabled
add_executable(FastSIMD_tests "test.cpp" "test_inputs.cpp")
target_link_libraries(FastSIMD_tests PRIVATE FastSIMD ${simd_test_libraries})

# Random inputs must be the same on every platform for a seed, stop arm64 compilers fusing the range maths into FMA
if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(FastSIMD_tests PRIVATE /clang:-ffp-contract=off)
elseif(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(FastSIMD_tests PRIVATE -ffp-contract=off)
endif()

# Fewer random inputs keeps ctest (and CI) quick, run FastSIMD_tests directly for a full run
set(FASTSIMD_CTEST_INPUT_COUNT 65536 CACHE STRING "FastSIMD random input values per test when run through ctest")
add_test(NAME FastSIMD_tests COMMAND FastSIMD_tests --count ${FASTSIMD_CTEST_INPUT_COUNT})

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  # Runs under node, which the Emscripten toolchain sets as the ctest emulator
  # EXIT_RUNTIME makes main's return value the exit code
  # Debug info makes the single file output over 1GB, larger than node can load, so it's left out when linking
  target_link_options(FastSIMD_tests PRIVATE -sALLOW_MEMORY_GROWTH=1 -sSINGLE_FILE -sEXIT_RUNTIME=1 $<$<CONFIG:Debug>:-g0>)

  # The toolchain uses whichever node is first on PATH, emsdk's own node supports the relaxed SIMD the relaxed tests use
  if(DEFINED ENV{EMSDK_NODE})
    set_target_properties(FastSIMD_tests PROPERTIES CROSSCOMPILING_EMULATOR "$ENV{EMSDK_NODE}")
  endif()

  # Single page to run the tests in a browser
  add_executable(FastSIMD_tests_web "test.cpp" "test_inputs.cpp")
  target_link_libraries(FastSIMD_tests_web PRIVATE FastSIMD ${simd_test_libraries})
  set_target_properties(FastSIMD_tests_web PROPERTIES SUFFIX ".html")
  target_link_options(FastSIMD_tests_web PRIVATE -sALLOW_MEMORY_GROWTH=1 -sSINGLE_FILE $<$<CONFIG:Debug>:-g0>)
endif()

# Enable Warnings
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #target_compile_options(test_simd PUBLIC /W4 /WX)
else()
  #target_compile_options(test_simd PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-undefined-func-template)
endif()
