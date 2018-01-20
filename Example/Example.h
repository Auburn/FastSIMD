#include "../FastSIMD/FS_Class.inl"
#ifdef FASTSIMD_INCLUDE_CHECK
#include __FILE__
#endif
#include "../FastSIMD/FS_Class.inl"
#pragma once

FASTSIMD_CLASS_DEFINITION( Example )
{
public:
    FASTSIMD_SET_SUPPORTED_SIMD_LEVELS( FastSIMD::Level_AVX2 | FastSIMD::Level_SSE41 | FastSIMD::Level_SSE2 | FastSIMD::Level_Scalar );

    FASTSIMD_CLASS_SETUP();

    FS_EXTERNAL_FUNC( void DoStuff( int* data ) );

    FS_EXTERNAL_FUNC( void DoArray( int* data0, int* data1, int size ) );
};
