#pragma once

#include "VecTools.h"
#include <cmath>

class FastSIMD_Scalar
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_Scalar;

    typedef float_t float32v;
    typedef int32_t int32v;

    typedef const float32v& float32v_arg;
    typedef const int32v& int32v_arg;

    FS_INLINE static float32v VecZero_f32()
    {
        return 0.0f;
    }

    FS_INLINE static int32v VecZero_i32()
    {
        return 0;
    }

    FS_INLINE static float32v VecIncremented_f32()
    {
        return 0.0f;
    }

    FS_INLINE static int32v VecIncremented_i32()
    {
        return 0;
    }

    FS_INLINE static float32v VecFill_f32( float_t a )
    {
        return a;
    }

    FS_INLINE static int32v VecFill_i32( int32_t a )
    {
        return a;
    }

    FS_INLINE static float32v Load_f32( void* p )
    {
        return *reinterpret_cast<float32v*>(p);
    }

    FS_INLINE static int32v Load_i32( void* p )
    {
        return *reinterpret_cast<int32v*>(p);
    }

    FS_INLINE static void Store_f32( void* p, float32v_arg a )
    {
        *reinterpret_cast<float32v*>(p) = a;
    }

    FS_INLINE static void Store_i32( void* p, int32v_arg a )
    {
        *reinterpret_cast<int32v*>(p) = a;
    }
};
