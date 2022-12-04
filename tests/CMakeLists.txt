
fastsimd_create_dispatch_library(test_simd SOURCES "test.inl" FEATURE_SETS Scalar SSE2 SSE41 AVX2_FMA AVX512_Baseline_FMA)

add_executable(test "test.cpp")
target_link_libraries(test PRIVATE FastSIMD test_simd)