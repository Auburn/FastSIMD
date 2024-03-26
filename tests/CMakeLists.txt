
fastsimd_create_dispatch_library(test_simd SOURCES "test.inl" FEATURE_SETS SCALAR SSE2 SSE41 AVX2 AVX512 NEON AARCH64 WASM)
fastsimd_create_dispatch_library(test_simd_relaxed RELAXED SOURCES "test.inl" FEATURE_SETS SCALAR SSE2 SSE41 AVX2 AVX512 NEON AARCH64 WASM)

add_executable(test "test.cpp")
target_link_libraries(test PRIVATE FastSIMD test_simd test_simd_relaxed)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  target_link_options(test PRIVATE -sALLOW_MEMORY_GROWTH=1 -sSINGLE_FILE)
endif()

# Enable Warnings
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #target_compile_options(test_simd PUBLIC /W4 /WX)
else()
  #target_compile_options(test_simd PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-undefined-func-template)
endif()
