#pragma once

#include "FastSIMD_Config.h"

namespace FastSIMD
{
    enum Level : unsigned int
    {
        Level_Null       = 0,       // Uninitilised
        Level_Scalar     = 1 <<  0, // 80386 instruction set (Not SIMD)
        Level_SSE        = 1 <<  1, // SSE (XMM) supported by CPU (not testing for O.S. support)
        Level_SSE2       = 1 <<  2, // SSE2
        Level_SSE3       = 1 <<  3, // SSE3
        Level_SSSE3      = 1 <<  4, // Supplementary SSE3 (SSSE3)
        Level_SSE41      = 1 <<  5, // SSE4.1
        Level_SSE42      = 1 <<  6, // SSE4.2
        Level_AVX        = 1 <<  7, // AVX supported by CPU and operating system
        Level_AVX2       = 1 <<  8, // AVX2
        Level_AVX512F    = 1 <<  9, // AVX512F
        Level_AVX512VL   = 1 << 10, // AVX512VL
        Level_AVX512BWDQ = 1 << 11, // AVX512BW, AVX512DQ
    };

    const unsigned int COMPILED_SIMD_LEVELS =
        Level_Scalar     * FASTSIMD_COMPILE_SCALAR    |
        Level_SSE        * FASTSIMD_COMPILE_SSE       |
        Level_SSE2       * FASTSIMD_COMPILE_SSE2      |
        Level_SSE3       * FASTSIMD_COMPILE_SSE3      |
        Level_SSSE3      * FASTSIMD_COMPILE_SSSE3     |
        Level_SSE41      * FASTSIMD_COMPILE_SSE41     |
        Level_SSE42      * FASTSIMD_COMPILE_SSE42     |
        Level_AVX        * FASTSIMD_COMPILE_AVX       |
        Level_AVX2       * FASTSIMD_COMPILE_AVX2      |
        Level_AVX512F    * FASTSIMD_COMPILE_AVX512F   |
        Level_AVX512VL   * FASTSIMD_COMPILE_AVX512VL  |
        Level_AVX512BWDQ * FASTSIMD_COMPILE_AVX512BWQD;

    Level CPUMaxSIMDLevel();

    template<typename T>
    T* NewSIMDClass( Level maxSIMDLevel );

    template<typename T>
    T* NewSIMDClass()
    {
        return NewSIMDClass<T>( CPUMaxSIMDLevel() );
    }

	template<typename CLASS_T, typename SIMD_T>
    struct ClassFactory
    {
        static_assert( SIMD_T::SIMD_Level & COMPILED_SIMD_LEVELS, "SIMD level is not being compiled, check FastSIMD_Config.h for FASTSIMD_COMPILED_SIMD_LEVELS" );
        static_assert( SIMD_T::SIMD_Level & CLASS_T::Supported_SIMD_Levels, "Class doesn't support this SIMD level, check class header for FASTSIMD_SET_SUPPORTED_SIMD_LEVELS" );

        static CLASS_T* Get();
    };
};
