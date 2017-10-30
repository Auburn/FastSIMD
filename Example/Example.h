#include "../FastSIMD/FS_Class.inl"
#ifdef FASTSIMD_INCLUDE_CHECK
#include __FILE__
#endif
#include "../FastSIMD/FS_Class.inl"
#pragma once

FASTSIMD_CLASS_DEFINITION( Example )
{
public:
    FASTSIMD_CLASS_TYPEDEFS();

    FS_FUNC_EXTERNAL( void DoStuff( int* data ) );

    FS_FUNC_EXTERNAL( void DoArray( int* data0, int* data1, int size ) );
};
