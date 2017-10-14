#pragma once

#include "FastSIMD_Config.h"

namespace FastSIMD
{
    enum Level
    {
        Level_Null       = -1, // Uninitilised
        Level_Scalar     =  0, // 80386 instruction set (Not SIMD)
        Level_SSE        =  1, // SSE (XMM) supported by CPU (not testing for O.S. support)
        Level_SSE2       =  2, // SSE2
        Level_SSE3       =  3, // SSE3
        Level_SSSE3      =  4, // Supplementary SSE3 (SSSE3)
        Level_SSE41      =  5, // SSE4.1
        Level_SSE42      =  6, // SSE4.2
        Level_AVX        =  7, // AVX supported by CPU and operating system
        Level_AVX2       =  8, // AVX2
        Level_AVX512F    =  9, // AVX512F
        Level_AVX512VL   = 10, // AVX512VL
        Level_AVX512BWDQ = 11, // AVX512BW, AVX512DQ
    };

    template<typename T>
    T* NewSIMDClass();

    template<typename T>
    T* NewSIMDClass(Level simdLevel);

    Level GetSIMDLevel();

};

