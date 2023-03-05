
function(fastsimd_add_feature_set_source simd_inl feature_set)
    set(feature_set_source "${simd_library_source_dir}/${simd_library_name}_${feature_set}.cpp")
    set(simd_inl_full "${CMAKE_CURRENT_LIST_DIR}/${simd_inl}")
    
    configure_file("${FastSIMD_SOURCE_DIR}/DispatchClass/cmake/feature_set_source.cpp.in" ${feature_set_source})
    target_sources(${simd_library_name} PRIVATE ${feature_set_source})
            
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # MSVC 32bit needs SSE2 flag for all SSE levels
        if(${feature_set} MATCHES "SSE[^(0-9)]" AND CMAKE_SIZEOF_VOID_P EQUAL 4) 
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "/arch:SSE2")
        
        elseif(${feature_set} MATCHES "AVX[^(0-9)]")
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "/arch:AVX")
        
        elseif(${feature_set} MATCHES AVX2)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "/arch:AVX2")

        elseif(${feature_set} MATCHES AVX512)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "/arch:AVX512")
        endif()
    else()
        if(${feature_set} MATCHES SSE2 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-msse2")
        
        elseif(${feature_set} MATCHES SSE3)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-msse3")
        
        elseif(${feature_set} MATCHES SSSE3)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-mssse3")
        
        elseif(${feature_set} MATCHES SSE41)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-msse4.1")
        
        elseif(${feature_set} MATCHES SSE42)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-msse4.2")
        
        elseif(${feature_set} MATCHES "AVX[^(0-9)]")
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-mavx")
        
        elseif(${feature_set} MATCHES AVX2)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")

        elseif(${feature_set} MATCHES AVX512)
            set_source_files_properties(${feature_set_source} PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512dq -mavx512vl -mavx512bw -mfma")
        endif()
    endif()
    
endfunction()

function(fastsimd_create_dispatch_library simd_library_name)
    
    cmake_parse_arguments(PARSE_ARGV 0 fastsimd_create_dispatch_library "" "" "SOURCES;FEATURE_SETS")

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
            AVX2_FMA
            AVX512_Baseline_FMA)
    endif()

    message(STATUS "FastSIMD: Created dispatch library \"${simd_library_name}\" with Feature Sets: ${fastsimd_create_dispatch_library_FEATURE_SETS}")

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

    if(CMAKE_COMPILER_IS_GNUCC)
        set_target_properties(${simd_library_name} PROPERTIES COMPILE_FLAGS "-Wno-ignored-attributes")
    endif()

    set(feature_set_list "")

    foreach(simd_inl ${fastsimd_create_dispatch_library_SOURCES})
        foreach(feature_set ${fastsimd_create_dispatch_library_FEATURE_SETS})
            list(APPEND feature_set_list "FastSIMD::FeatureSet::${feature_set}")      
            fastsimd_add_feature_set_source(${simd_inl} ${feature_set})
        endforeach()  
    endforeach()  

    # Create array of compiled feature sets for lookup in FastSIMD::New()
    string(REPLACE ";" ",\n" feature_set_list "${feature_set_list}")
    configure_file("${FastSIMD_SOURCE_DIR}/DispatchClass/cmake/simd_lib_config.h.in" "${simd_library_source_dir}/include/FastSIMD/${simd_library_name}_config.h")

endfunction()
