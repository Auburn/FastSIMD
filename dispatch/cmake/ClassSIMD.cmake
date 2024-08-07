
function(fastsimd_add_feature_set_source simd_inl feature_set is_relaxed)
    set(feature_set_source "${simd_library_source_dir}/${simd_library_name}_${feature_set}.cpp")
    set(simd_inl_full "${CMAKE_CURRENT_LIST_DIR}/${simd_inl}")

    configure_file("${FastSIMD_SOURCE_DIR}/dispatch/cmake/feature_set_source.cpp.in" ${feature_set_source})
    target_sources(${simd_library_name} PRIVATE ${feature_set_source})

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # MSVC 32bit needs SSE2 flag for all SSE levels
        if(${feature_set} MATCHES "SSE[^(0-9)]" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS /arch:SSE2)

        elseif(${feature_set} MATCHES "AVX[^(0-9)]")
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS /arch:AVX)

        elseif(${feature_set} MATCHES AVX2)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS /arch:AVX2)

        elseif(${feature_set} MATCHES AVX512)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS /arch:AVX512)
        endif()
    else()
        if(${feature_set} MATCHES SSE2 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -msse2)

        elseif(${feature_set} MATCHES SSE3)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -msse3)

        elseif(${feature_set} MATCHES SSSE3)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mssse3)

        elseif(${feature_set} MATCHES SSE41)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -msse4.1)

        elseif(${feature_set} MATCHES SSE42)
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -msse4.2)

        elseif(${feature_set} MATCHES "AVX[^(0-9)]")
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mavx)

        elseif(${feature_set} MATCHES AVX2)
            if(is_relaxed)
                set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mfma)
            else()
                set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mno-fma) 
            endif()
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mavx2)

        elseif(${feature_set} MATCHES AVX512)
            if(is_relaxed)
                set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mfma)
            else()
                set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mno-fma)                
            endif()
            set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mavx512f -mavx512dq -mavx512vl -mavx512bw)

        elseif(${feature_set} MATCHES WASM)
            if(is_relaxed)
                set_property(SOURCE ${feature_set_source} APPEND PROPERTY COMPILE_OPTIONS -mrelaxed-simd)
            endif()
        endif()
    endif()

endfunction()

function(fastsimd_create_dispatch_library simd_library_name)

    cmake_parse_arguments(PARSE_ARGV 0 fastsimd_create_dispatch_library "RELAXED" "" "SOURCES;FEATURE_SETS")

    list(LENGTH fastsimd_create_dispatch_library_FEATURE_SETS FEATURE_SET_COUNT)
    list(LENGTH fastsimd_create_dispatch_library_SOURCES SOURCES_COUNT)

    if(SOURCES_COUNT EQUAL 0)
        message(FATAL_ERROR "FastSIMD: \"${simd_library_name}\" No SOURCES specified, example usage: fastsimd_create_dispatch_library(example_simd SOURCES \"example.inl\")")
    endif()

    if(FEATURE_SET_COUNT EQUAL 0)
        message("FastSIMD: \"${simd_library_name}\" No FEATURE_SETS specified, using default feature sets")
        set(fastsimd_create_dispatch_library_FEATURE_SETS
            SSE2
            SSE41
            AVX2
            AVX512
            NEON
            AARCH64
            WASM)
    endif()

    add_library(${simd_library_name} OBJECT)

    set(simd_library_source_dir "${CMAKE_CURRENT_BINARY_DIR}/fastsimd/${simd_library_name}")

    target_compile_definitions(${simd_library_name} PRIVATE FASTSIMD_EXPORT FASTSIMD_LIBRARY_NAME=${simd_library_name})
    target_link_libraries(${simd_library_name} PRIVATE FastSIMD)

    target_include_directories(${simd_library_name} PUBLIC
        $<BUILD_INTERFACE:${simd_library_source_dir}/include>
        $<INSTALL_INTERFACE:include>)

    if(BUILD_SHARED_LIBS)
        set_target_properties(${simd_library_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set_target_properties(${simd_library_name} PROPERTIES COMPILE_FLAGS "-Wno-ignored-attributes")
    endif()

    if(MINGW)
        target_compile_options(${simd_library_name} PRIVATE -Wa,-muse-unaligned-vector-move)
    endif()

    if(fastsimd_create_dispatch_library_RELAXED)
        target_compile_definitions(${simd_library_name} PUBLIC FASTSIMD_IS_RELAXED=1)
        set(relaxed_log_msg " (RELAXED)")
    endif()

    set(feature_set_list "")
    set(feature_set_list_debug "")

    foreach(simd_inl ${fastsimd_create_dispatch_library_SOURCES})
        foreach(feature_set ${fastsimd_create_dispatch_library_FEATURE_SETS})
            if(DEFINED CMAKE_OSX_ARCHITECTURES AND NOT "${feature_set}" STREQUAL "SCALAR")
                # Loop through OSX arches and test compile on each separately
                foreach(CMAKE_OSX_ARCHITECTURES ${CMAKE_OSX_ARCHITECTURES})
                    #message(STATUS "${CMAKE_OSX_ARCHITECTURES} ${feature_set}")
                    try_compile(
                        compile_result_unused
                        "${CMAKE_BINARY_DIR}"
                        "${FastSIMD_SOURCE_DIR}/cmake/ArchDetect.cpp"
                        OUTPUT_VARIABLE COMPILE_OUTPUT
                        COMPILE_DEFINITIONS -DTEST_FEATURE_SET_ACTIVE=${feature_set}
                    )

                    #message(STATUS ${COMPILE_OUTPUT})
                    if ("${COMPILE_OUTPUT}" MATCHES "FASTSIMD_ARCH<([^\"=]+)=([^>]+)")
                        set(feature_arch_detect "FASTSIMD_CURRENT_ARCH_IS( ${CMAKE_MATCH_1} )")
                        fastsimd_add_feature_set_source(${simd_inl} ${feature_set} ${fastsimd_create_dispatch_library_RELAXED})
                        string(APPEND feature_set_list "#if ${feature_arch_detect}\n,FastSIMD::FeatureSet::${feature_set}\n#endif\n" )
                        list(APPEND feature_set_list_debug "${feature_set}")
                        break()
                    endif()
                endforeach()
            else()
                try_compile(
                    compile_result_unused
                    "${CMAKE_BINARY_DIR}"
                    "${FastSIMD_SOURCE_DIR}/cmake/ArchDetect.cpp"
                    OUTPUT_VARIABLE COMPILE_OUTPUT
                    COMPILE_DEFINITIONS -DTEST_FEATURE_SET_ACTIVE=${feature_set}
                )

                #message(STATUS ${COMPILE_OUTPUT})
                if ("${COMPILE_OUTPUT}" MATCHES "FASTSIMD_ARCH<([^\"=]+)=([^>]+)")
                    set(feature_arch_detect "1")
                    fastsimd_add_feature_set_source(${simd_inl} ${feature_set} ${fastsimd_create_dispatch_library_RELAXED})
                    string(APPEND feature_set_list ",FastSIMD::FeatureSet::${feature_set}\n" )
                    list(APPEND feature_set_list_debug "${feature_set}")
                endif()
            endif()
        endforeach()
    endforeach()

    # Create array of compiled feature sets for lookup in FastSIMD::New()
    configure_file("${FastSIMD_SOURCE_DIR}/dispatch/cmake/simd_lib_config.h.in" "${simd_library_source_dir}/include/FastSIMD/${simd_library_name}_config.h")

    message(STATUS "FastSIMD: Created dispatch library \"${simd_library_name}\" with Feature Sets${relaxed_log_msg}: ${feature_set_list_debug}")

endfunction()
