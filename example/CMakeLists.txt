
fastsimd_create_simd_library(example_simd "example.inl")

add_executable(test "main.cpp")
target_link_libraries(test PRIVATE FastSIMD example_simd)