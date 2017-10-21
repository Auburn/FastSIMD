#pragma once

#include <intrin.h>

#include "VecTools.h"

struct SSE2_f32x4
{
    FASTSIMD_INTERNAL_TYPE_SET( SSE2_f32x4, __m128 );

    FS_INLINE SSE2_f32x4& operator+=( const SSE2_f32x4& rhs )
    {
        *this = _mm_add_ps( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator-=( const SSE2_f32x4& rhs )
    {
        *this = _mm_sub_ps( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator*=( const SSE2_f32x4& rhs )
    {
        *this = _mm_mul_ps( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator/=( const SSE2_f32x4& rhs )
    {
        *this = _mm_div_ps( *this, rhs );
        return *this;
    }
};

FASTSIMD_INTERNAL_OPERATORS_FLOAT( SSE2_f32x4 )


struct SSE2_i32x4
{
    FASTSIMD_INTERNAL_TYPE_SET( SSE2_i32x4, __m128i );

    FS_INLINE SSE2_i32x4& operator+=( const SSE2_i32x4& rhs )
    {
        *this = _mm_add_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator-=( const SSE2_i32x4& rhs )
    {
        *this = _mm_sub_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator*=( const SSE2_i32x4& rhs )
    {
        __m128 tmp1 = _mm_castsi128_ps( _mm_mul_epu32( *this, rhs ) ); /* mul 2,0*/
        __m128 tmp2 = _mm_castsi128_ps( _mm_mul_epu32( _mm_srli_si128( *this, 4 ), _mm_srli_si128( rhs, 4 ) ) ); /* mul 3,1 */
        *this = _mm_shuffle_epi32( _mm_castps_si128( _mm_shuffle_ps( tmp1, tmp2, _MM_SHUFFLE( 2, 0, 2, 0 ) ) ), _MM_SHUFFLE( 3, 1, 2, 0 ) );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator&=( const SSE2_i32x4& rhs )
    {
        *this = _mm_and_si128( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator|=( const SSE2_i32x4& rhs )
    {
        *this = _mm_or_si128( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator^=( const SSE2_i32x4& rhs )
    {
        *this = _mm_xor_si128( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator>>=( int32_t rhs )
    {
        *this = _mm_srai_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator<<=( int32_t rhs )
    {
        *this = _mm_slli_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE SSE2_i32x4 operator~() const
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128i neg1 = _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() );
#else
        __m128i neg1 = _mm_set1_epi32( -1 );
#endif
        return _mm_xor_si128( *this, neg1 );        
    }
};

FASTSIMD_INTERNAL_OPERATORS_INT( SSE2_i32x4, int32_t )


class FastSIMD_SSE2
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_SSE2;
    static const size_t VectorBits = 128;

    typedef SSE2_f32x4 float32v;
    typedef SSE2_i32x4 int32v;
    typedef SSE2_i32x4 mask32v;

    typedef const float32v& float32v_arg;
    typedef const int32v&   int32v_arg;
    typedef const mask32v&  mask32v_arg;

    FS_INLINE static float32v VecZero_f32()
    {
        return _mm_setzero_ps();
    }

    FS_INLINE static int32v VecZero_i32()
    {
        return _mm_setzero_si128();
    }

    FS_INLINE static float32v VecIncremented_f32()
    {
        return _mm_set_ps( 3.0f, 2.0f, 1.0f, 0.0f );
    }

    FS_INLINE static int32v VecIncremented_i32()
    {
        return _mm_set_epi32( 3, 2, 1, 0 );
    }

    FS_INLINE static float32v VecFill_f32( float_t a )
    {
        return _mm_set1_ps( a );
    }

    FS_INLINE static int32v VecFill_i32( int32_t a )
    {
        return _mm_set1_epi32( a );
    }

    FS_INLINE static float32v Vec4_f32( float_t f0, float_t f1, float_t f2, float_t f3 )
    {
        return _mm_set_ps( f3, f2, f1, f0 );
    }

    FS_INLINE static int32v Vec4_i32( int32_t i0, int32_t i1, int32_t i2, int32_t i3 )
    {
        return _mm_set_epi32( i3, i2, i1, i0 );
    }

    FS_INLINE static float32v Load_f32( void* p )
    {
        return _mm_loadu_ps( reinterpret_cast<float_t*>(p) );
    }

    FS_INLINE static int32v Load_i32( void* p )
    {
        return _mm_loadu_si128( reinterpret_cast<__m128i*>(p) );
    }

    FS_INLINE static void Store_f32( void* p, float32v_arg a )
    {
        _mm_store_ps( reinterpret_cast<float_t*>(p), a );
    }

    FS_INLINE static void Store_i32( void* p, int32v_arg a )
    {
        _mm_store_si128( reinterpret_cast<__m128i*>(p), a );
    }

    FS_INLINE static float32v Casti32_f32( int32v_arg a )
    {
        return _mm_castsi128_ps( a );
    }

    FS_INLINE static int32v Castf32_i32( float32v_arg a )
    {
        return _mm_castps_si128( a );
    }

    FS_INLINE static float32v Converti32_f32( int32v_arg a )
    {
        return _mm_cvtepi32_ps( a );
    }

    FS_INLINE static int32v Convertf32_i32( float32v_arg a )
    {
        return _mm_cvtps_epi32( a );
    }

    FS_INLINE static int32v AndNot_i32( int32v_arg a, int32v_arg b )
    {
        return _mm_andnot_si128( b, a );
    }

    FS_INLINE static mask32v Equal_i32( int32v_arg a, int32v_arg b )
    {
        return _mm_cmpeq_epi32( a, b );
    }

    FS_INLINE static mask32v GreaterThan_i32( int32v_arg a, int32v_arg b )
    {
        return _mm_cmpgt_epi32( a, b );
    }

    FS_INLINE static mask32v LessThan_i32( int32v_arg a, int32v_arg b )
    {
        return _mm_cmplt_epi32( a, b );
    }

    FS_INLINE static int32v Select_i32( int32v_arg a, int32v_arg b, mask32v_arg m )
    {
        return  _mm_or_si128( _mm_and_si128( m, a ), _mm_andnot_si128( m, b ) );
    }

    FS_INLINE static int32v Max_i32( int32v_arg a, int32v_arg b )
    {
        return Select_i32( a, b, GreaterThan_i32( a, b ) );
    }

    FS_INLINE static int32v Min_i32( int32v_arg a, int32v_arg b )
    {
        return Select_i32( a, b, LessThan_i32( a, b ) );
    }

    FS_INLINE static float32v And_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_and_ps( a, b );
    }

    FS_INLINE static float32v AndNot_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_andnot_ps( b, a );
    }

    FS_INLINE static float32v Or_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_or_ps( a, b );
    }

    FS_INLINE static float32v Xor_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_xor_ps( a, b );
    }

    FS_INLINE static float32v Not_f32( float32v_arg a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128i neg1 = _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() );
#else
        __m128i neg1 = _mm_set1_epi32( -1 );
#endif
        return _mm_xor_ps( a, _mm_castsi128_ps( neg1 ) );
    }

    FS_INLINE static float32v Max_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_max_ps( a, b );
    }

    FS_INLINE static float32v Min_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_min_ps( a, b );
    }

    FS_INLINE static mask32v GreaterThan_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_castps_si128( _mm_cmpgt_ps( a, b ) );
    }

    FS_INLINE static mask32v LessThan_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_castps_si128( _mm_cmplt_ps( a, b ) );
    }

    FS_INLINE static mask32v GreaterEqualThan_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_castps_si128( _mm_cmpge_ps( a, b ) );
    }

    FS_INLINE static mask32v LessEqualThan_f32( float32v_arg a, float32v_arg b )
    {
        return _mm_castps_si128( _mm_cmple_ps( a, b ) );
    }

    FS_INLINE static float32v Abs_f32( float32v_arg a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128i intMax = _mm_srli_epi32( _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() ), 1 );
#else
        __m128i intMax = _mm_set1_epi32( 0x7FFFFFFF );
#endif
        return _mm_and_ps( a, _mm_castsi128_ps( intMax ) );
    }

    FS_INLINE static float32v InvSqrt_f32( float32v_arg a )
    {
        return _mm_rsqrt_ps( a );
    }

// Floor, Ceil, Round: http://dss.stephanierct.com/DevBlog/?p=8

    FS_INLINE static float32v Floor_f32( float32v_arg a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128 f1 = _mm_castsi128_ps( _mm_slli_epi32( _mm_srli_epi32( _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() ), 25 ), 23 ) );
#else
        __m128 f1 = _mm_set1_ps( 1.0f );
#endif
        __m128 fval = _mm_cvtepi32_ps( _mm_cvttps_epi32( a ) );

        return _mm_sub_ps( fval, _mm_and_ps( _mm_cmplt_ps( a, fval ), f1 ) );
    }

    FS_INLINE static float32v Ceil_f32( float32v_arg a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128 f1 = _mm_castsi128_ps( _mm_slli_epi32( _mm_srli_epi32( _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() ), 25 ), 23 ) );
#else
        __m128 f1 = _mm_set1_ps( 1.0f );
#endif
        __m128 fval = _mm_cvtepi32_ps( _mm_cvttps_epi32( a ) );

        return _mm_add_ps( fval, _mm_and_ps( _mm_cmplt_ps( a, fval ), f1 ) );
    }


    FS_INLINE static float32v Round_f32( float32v_arg a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        __m128 nearest2 = _mm_castsi128_ps( _mm_srli_epi32( _mm_cmpeq_epi32( _mm_setzero_si128(), _mm_setzero_si128() ), 2 ) );
#else
        __m128 nearest2 = _mm_set1_ps( 1.99999988079071044921875f );
#endif
        __m128 aTrunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( a ) );       // truncate a
        __m128 rmd = _mm_sub_ps( a, aTrunc );                           // get remainder
        __m128 rmd2 = _mm_mul_ps( rmd, nearest2 );                      // mul remainder by near 2 will yield the needed offset
        __m128 rmd2Trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( rmd2 ) ); // after being truncated of course
        return _mm_add_ps( aTrunc, rmd2Trunc );        
    }


    FS_INLINE static float32v_arg Select_f32( float32v_arg a, float32v_arg b, mask32v_arg m )
    {
        __m128 mf = _mm_castsi128_ps( m );

        return  _mm_or_ps( _mm_and_ps( mf, a ), _mm_andnot_ps( mf, b ) );
    }
};
