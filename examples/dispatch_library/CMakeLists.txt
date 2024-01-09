
fastsimd_create_dispatch_library(example_simd SOURCES "example.inl")

add_executable(example_dispatch_library "main.cpp")
target_link_libraries(example_dispatch_library PRIVATE FastSIMD example_simd)