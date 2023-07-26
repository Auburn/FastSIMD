
fastsimd_create_dispatch_library(example_simd SOURCES "example.inl")

add_executable(example "main.cpp")
target_link_libraries(example PRIVATE FastSIMD example_simd)