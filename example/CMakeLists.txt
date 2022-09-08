
fastsimd_create_simd_library(example_simd SOURCES "example.inl" FEATURE_SETS SSE2 SSE41 AVX2_FMA AVX512_Baseline_FMA)

add_executable(example "main.cpp")
target_link_libraries(example PRIVATE FastSIMD example_simd)