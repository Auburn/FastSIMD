#pragma once

#include "VecTools.h"
#include <algorithm>

class FastSIMD_Scalar
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_Scalar;

    typedef float   float32v;
    typedef int32_t int32v;
    typedef bool    mask32v;

    // Vector builders

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

    FS_INLINE static float32v VecFill_f32( float a )
    {
        return a;
    }

    FS_INLINE static int32v VecFill_i32( int32_t a )
    {
        return a;
    }

    // Load

    FS_INLINE static float32v Load_f32( void const* p )
    {
        return *reinterpret_cast<float32v const*>(p);
    }

    FS_INLINE static int32v Load_i32( void const* p )
    {
        return *reinterpret_cast<int32v const*>(p);
    }

    // Store

    FS_INLINE static void Store_f32( void* p, float32v a )
    {
        *reinterpret_cast<float32v*>(p) = a;
    }

    FS_INLINE static void Store_i32( void* p, int32v a )
    {
        *reinterpret_cast<int32v*>(p) = a;
    }

    // Cast

    FS_INLINE static float32v Casti32_f32( int32v a )
    {
        union
        {
            int32_t i;
            float   f;
        } u;

        u.i = a;
        return u.f;
    }

    FS_INLINE static int32v Castf32_i32( float32v a )
    {
        union
        {
            int32_t i;
            float   f;
        } u;

        u.f = a;
        return u.i;
    }

    // Convert

    FS_INLINE static float32v Converti32_f32( int32v a )
    {
        return static_cast<float32v>( a );
    }

    FS_INLINE static int32v Convertf32_i32( float32v a )
    {
        return static_cast<int32v>( a );
    }

    // Comparisons

    FS_INLINE static mask32v GreaterThan_f32( float32v a, float32v b )
    {
        return a > b ? -1 : 0;
    }

    FS_INLINE static mask32v LessThan_f32( float32v a, float32v b )
    {
        return a < b ? -1 : 0;
    }

    FS_INLINE static mask32v GreaterEqualThan_f32( float32v a, float32v b )
    {
        return a >= b ? -1 : 0;
    }

    FS_INLINE static mask32v LessEqualThan_f32( float32v a, float32v b )
    {
        return a <= b ? -1 : 0;
    }

    FS_INLINE static mask32v Equal_i32( int32v a, int32v b )
    {
        return a == b ? -1 : 0;
    }

    FS_INLINE static mask32v GreaterThan_i32( int32v a, int32v b )
    {
        return a > b ? -1 : 0;
    }

    FS_INLINE static mask32v LessThan_i32( int32v a, int32v b )
    {
        return a < b ? -1 : 0;
    }

    // Select

    FS_INLINE static float32v Select_f32( mask32v m, float32v a, float32v b )
    {
        return m ? a : b;
    }

    FS_INLINE static int32v Select_i32( mask32v m, int32v a, int32v b )
    {
        return m ? a : b;
    }

    // Min, Max

    FS_INLINE static float32v Min_f32( float32v a, float32v b )
    {
        return std::min( a, b );
    }

    FS_INLINE static float32v Max_f32( float32v a, float32v b )
    {
        return std::max( a, b );
    }

    FS_INLINE static int32v Min_i32( int32v a, int32v b )
    {
        return std::min( a, b );
    }

    FS_INLINE static int32v Max_i32( int32v a, int32v b )
    {
        return std::max( a, b );
    }

    // Bitwise

    FS_INLINE static float32v BitwiseAnd_f32( float32v a, float32v b )
    {
        return Casti32_f32( Castf32_i32( a ) & Castf32_i32( b ) );
    }

    FS_INLINE static float32v BitwiseOr_f32( float32v a, float32v b )
    {
        return Casti32_f32( Castf32_i32( a ) | Castf32_i32( b ) );
    }

    FS_INLINE static float32v BitwiseXor_f32( float32v a, float32v b )
    {
        return Casti32_f32( Castf32_i32( a ) ^ Castf32_i32( b ) );
    }

    FS_INLINE static float32v BitwiseNot_f32( float32v a )
    {
        return Casti32_f32( ~Castf32_i32( a ) );
    }

    FS_INLINE static float32v BitwiseAndNot_f32( float32v a, float32v b )
    {
        return Casti32_f32( Castf32_i32( a ) & ~Castf32_i32( b ) );
    }

    FS_INLINE static int32v BitwiseAndNot_i32( int32v a, int32v b )
    {
        return a & ~b;
    }

    // Abs

    FS_INLINE static float32v Abs_f32( float32v a )
    {
        return std::abs( a );
    }

    FS_INLINE static int32v Abs_i32( int32v a )
    {
        return std::abs( a );
    }

    // Float math

    FS_INLINE static float32v Sqrt_f32( float32v a )
    {
        return std::sqrt( a );
    }

    FS_INLINE static float32v InvSqrt_f32( float32v a )
    {
        float xhalf = 0.5f * a;
        int32_t i = Castf32_i32( a );
        i = 0x5f3759df - ( i >> 1 );
        a = Casti32_f32( i );
        a = a * ( 1.5f - xhalf * a * a );
        return a;
    }

    // Floor, Ceil, Round: http://dss.stephanierct.com/DevBlog/?p=8
    
    FS_INLINE static float32v Floor_f32( float32v a )
    {
        return std::floor( a );
    }

    FS_INLINE static float32v Ceil_f32( float32v a )
    {
        return std::ceil( a );
    }

    FS_INLINE static float32v Round_f32( float32v a )
    {
        return std::round( a );
    }
};
