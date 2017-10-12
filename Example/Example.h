#ifndef NOISE_H 

#include "FastSIMD/FS_Class.inc"
#ifdef FASTSIMD_INCLUDE_CHECK
#include __FILE__
#endif 
#include "FastSIMD/FS_Class.inc"

#define NOISE_H

FASTSIMD_CLASS_DEFINITION(Example)
{
public:
    FASTSIMD_CLASS_INIT()

    FS_FUNC_EXTERNAL(void DoStuff(int* data));

    //FS_INTERNAL(FS_CLASS(Noise)() {});
};

#endif
