#ifndef NOISE_H 

#include "../FastSIMD/FS_Class.inl"
#ifdef FASTSIMD_INCLUDE_CHECK
#include __FILE__
#endif 
#include "../FastSIMD/FS_Class.inl"

#define NOISE_H

FASTSIMD_CLASS_DEFINITION(Example)
{
public:
    FASTSIMD_CLASS_INIT()

    FS_FUNC_EXTERNAL( void DoStuff( int* data ) );

    FS_FUNC_EXTERNAL( void DoArray( int* data0, int* data1, int size ) );

};

#endif