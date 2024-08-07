function(target_architecture output_arch output_arch_ver)
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        set(ARCH "${CMAKE_OSX_ARCHITECTURES}")
        set(ARCH_VER unknown)
    else()

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of ifdefs that selects a
        # particular #error preprocessor directive whose message string contains the
        # target architecture. The program will always fail to compile (both because
        # file is not a valid C program, and obviously because of the presence of the
        # #error preprocessor directives... but by exploiting the preprocessor in this
        # way, we can detect the correct target architecture even when cross-compiling,
        # since the program itself never needs to be run (only the compiler/preprocessor)
        try_compile(
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${FastSIMD_SOURCE_DIR}/cmake/ArchDetect.cpp"
            OUTPUT_VARIABLE COMPILE_OUTPUT
        )
        #message(STATUS ${COMPILE_OUTPUT})

        # Parse the architecture name from the compiler output
        if ("${COMPILE_OUTPUT}" MATCHES "FASTSIMD_ARCH<([^\"=]+)=([^>]+)")
            set(ARCH "${CMAKE_MATCH_1}")
            set(ARCH_VER "${CMAKE_MATCH_2}")  

        else()       
            set(ARCH unknown)
            set(ARCH_VER SCALAR)   
        endif()
    endif()
    
    message(STATUS "FastSIMD: Detected arch \"${ARCH}\" \"${ARCH_VER}\"")
    set(${output_arch} "${ARCH}" PARENT_SCOPE)
    set(${output_arch_ver} "${ARCH_VER}" PARENT_SCOPE)
endfunction()