
fastsimd_create_dispatch_library(test_simd SOURCES "test.inl" FEATURE_SETS SCALAR SSE2 SSE41 AVX2_FMA AVX512_FMA NEON_FMA AARCH64_FMA WASM)

add_executable(test "test.cpp")
target_link_libraries(test PRIVATE FastSIMD test_simd)


# Enable Warnings
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #target_compile_options(test_simd PUBLIC /W4 /WX)
else()
  #target_compile_options(test_simd PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-undefined-func-template)
endif()
