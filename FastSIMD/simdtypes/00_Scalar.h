#pragma once

#include "base.h"
#include <cmath>

class FastSIMD_Scalar
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_Scalar;

    typedef float_t float32_v;
    typedef int32_t int32_v;

    typedef const float32_v& float32_v_arg;
    typedef const int32_v&   int32_v_arg;

    static float32_v SetAll_f32( float_t a )
    {
        return a;
    }

    static int32_v SetAll_i32( int32_t a )
    {
        return a;
    }

    static void Store_f32( float* p, float32_v_arg a )
    {
        *p = a;
    }

    static void Store_i32( void* p, int32_v_arg a )
    {
        *static_cast<int32_v*>(p) = a;
    }
};
