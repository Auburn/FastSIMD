add_library(FastSIMD_DispatchClass INTERFACE)

target_link_libraries(FastSIMD_DispatchClass INTERFACE FastSIMD)

target_include_directories(FastSIMD_DispatchClass INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(cmake/ClassSIMD.cmake)
