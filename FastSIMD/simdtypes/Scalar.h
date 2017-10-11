#pragma once


#include "base.h"
#include <cmath>

class FastSIMD_Scalar
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_Scalar;

    typedef float_t v_f32;
    typedef int32_t v_i32;

    typedef const v_f32& v_f32_arg;
    typedef const v_i32& v_i32_arg;

    static v_f32 Set1_f32(float a)
    {
        return a;
    }

    static v_i32 Set1_i32(int32_t a)
    {
        return a;
    }

    static void Store_f32(float* p, v_f32_arg a)
    {
        *p = a;
    }

    static void Store_i32(void* p, v_i32_arg a)
    {
        *static_cast<v_i32*>(p) = a;
    }
};
