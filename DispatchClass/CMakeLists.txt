add_library(FastSIMD_DispatchClass INTERFACE)

target_include_directories(FastSIMD_DispatchClass INTERFACE "include")

target_link_libraries(FastSIMD_DispatchClass INTERFACE FastSIMD)

include(cmake/ClassSIMD.cmake)
