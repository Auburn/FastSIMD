#pragma once

#include <intrin.h>

#include "VecTools.h"


struct AVX_f32x4
{
    FASTSIMD_INTERNAL_TYPE_SET( AVX_f32x4, __m256 );

    FS_INLINE AVX_f32x4& operator+=( const AVX_f32x4& rhs )
    {
        *this = _mm256_add_ps( *this, rhs );
        return *this;
    }

    FS_INLINE AVX_f32x4& operator-=( const AVX_f32x4& rhs )
    {
        *this = _mm256_sub_ps( *this, rhs );
        return *this;
    }

    FS_INLINE AVX_f32x4& operator*=( const AVX_f32x4& rhs )
    {
        *this = _mm256_mul_ps( *this, rhs );
        return *this;
    }

    FS_INLINE AVX_f32x4& operator/=( const AVX_f32x4& rhs )
    {
        *this = _mm256_div_ps( *this, rhs );
        return *this;
    }

    FS_INLINE AVX_f32x4 operator-() const
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        const __m256i minInt = _mm256_slli_epi32( _mm256_cmpeq_epi32( _mm256_setzero_si256(), _mm256_setzero_si256() ), 31 );
#else
        const __m256i minInt = _mm256_set1_epi32( -2147483648 );
#endif
        return _mm256_xor_ps( *this, _mm256_castsi256_ps( minInt ) );
    }
};

FASTSIMD_INTERNAL_OPERATORS_FLOAT( AVX_f32x4 )


struct AVX2_i32x4
{
    FASTSIMD_INTERNAL_TYPE_SET(AVX2_i32x4, __m256i);

    FS_INLINE AVX2_i32x4& operator+=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_add_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator-=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_sub_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator*=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_mullo_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator&=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_and_si256( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator|=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_or_si256( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator^=( const AVX2_i32x4& rhs )
    {
        *this = _mm256_xor_si256( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator>>=( int32_t rhs )
    {
        *this = _mm256_srai_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4& operator<<=( int32_t rhs )
    {
        *this = _mm256_slli_epi32( *this, rhs );
        return *this;
    }

    FS_INLINE AVX2_i32x4 operator~() const
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        const __m256i neg1 = _mm256_cmpeq_epi32( _mm256_setzero_si256(), _mm256_setzero_si256() );
#else
        const __m256i neg1 = _mm256_set1_epi32( -1 );
#endif
        return _mm256_xor_si256( *this, neg1 );
    }

    FS_INLINE AVX2_i32x4 operator-() const
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        const __m256i minInt = _mm256_slli_epi32( _mm256_cmpeq_epi32( _mm256_setzero_si256(), _mm256_setzero_si256() ), 31 );
#else
        const __m256i minInt = _mm256_set1_epi32( -2147483648 );
#endif
        return _mm256_xor_si256( *this, minInt );
    }
};

FASTSIMD_INTERNAL_OPERATORS_INT( AVX2_i32x4, int32_t )

template<FastSIMD::Level LEVEL_T>
class FastSIMD_AVX_T
{
public:
    static_assert(LEVEL_T >= FastSIMD::Level_AVX && LEVEL_T <= FastSIMD::Level_AVX2, "Cannot create template with unsupported SIMD level");

    static const FastSIMD::Level SIMD_Level = LEVEL_T;
    static const size_t VectorBits = 256;

    typedef AVX_f32x4  float32v;
    typedef AVX2_i32x4 int32v;
    typedef AVX2_i32x4 mask32v;

    // Vector builders

    FS_INLINE static float32v VecZero_f32()
    {
        return _mm256_setzero_ps();
    }

    FS_INLINE static int32v VecZero_i32()
    {
        return _mm256_setzero_si256();
    }

    FS_INLINE static float32v VecIncremented_f32()
    {
        return _mm256_set_ps( 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f );
    }

    FS_INLINE static int32v VecIncremented_i32()
    {
        return _mm256_set_epi32( 7, 6, 5, 4, 3, 2, 1, 0 );
    }

    FS_INLINE static float32v VecFill_f32( float a )
    {
        return _mm256_set1_ps( a );
    }

    FS_INLINE static int32v VecFill_i32( int32_t a )
    {
        return _mm256_set1_epi32( a );
    }

    FS_INLINE static float32v Vec8_f32( float f0, float f1, float f2, float f3, float f4, float f5, float f6, float f7 )
    {
        return _mm256_set_ps( f7, f6, f5, f4, f3, f2, f1, f0 );
    }

    FS_INLINE static int32v Vec8_i32( int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7 )
    {
        return _mm256_set_epi32( i7, i6, i5, i4, i3, i2, i1, i0 );
    }

    // Load

    FS_INLINE static float32v Load_f32( void const* p )
    {
        return _mm256_loadu_ps( reinterpret_cast<float const*>(p) );
    }

    FS_INLINE static int32v Load_i32( void const* p )
    {
        return _mm256_loadu_si256( reinterpret_cast<__m256i const*>(p) );
    }

    // Store

    FS_INLINE static void Store_f32( void* p, float32v a )
    {
        _mm256_storeu_ps( reinterpret_cast<float*>(p), a );
    }

    FS_INLINE static void Store_i32( void* p, int32v a )
    {
        _mm256_storeu_si256( reinterpret_cast<__m256i*>(p), a );
    }

    // Cast

    FS_INLINE static float32v Casti32_f32( int32v a )
    {
        return _mm256_castsi256_ps( a );
    }

    FS_INLINE static int32v Castf32_i32( float32v a )
    {
        return _mm256_castps_si256( a );
    }

    // Convert

    FS_INLINE static float32v Converti32_f32( int32v a )
    {
        return _mm256_cvtepi32_ps( a );
    }

    FS_INLINE static int32v Convertf32_i32( float32v a )
    {
        return _mm256_cvtps_epi32( a );
    }

    // Comparisons

    FS_INLINE static mask32v Equal_f32( float32v a, float32v b )
    {
        return _mm256_castps_si256( _mm256_cmp_ps( a, b, _CMP_EQ_OS ) );
    }

    FS_INLINE static mask32v GreaterThan_f32( float32v a, float32v b )
    {
        return _mm256_castps_si256( _mm256_cmp_ps( a, b, _CMP_GT_OS ) );
    }

    FS_INLINE static mask32v LessThan_f32( float32v a, float32v b )
    {
        return _mm256_castps_si256( _mm256_cmp_ps( a, b, _CMP_LT_OS ) );
    }

    FS_INLINE static mask32v GreaterEqualThan_f32( float32v a, float32v b )
    {
        return _mm256_castps_si256( _mm256_cmp_ps( a, b, _CMP_GE_OS ) );
    }

    FS_INLINE static mask32v LessEqualThan_f32( float32v a, float32v b )
    {
        return _mm256_castps_si256( _mm256_cmp_ps( a, b, _CMP_LE_OS ) );
    }

    FS_INLINE static mask32v Equal_i32( int32v a, int32v b )
    {
        return _mm256_cmpeq_epi32( a, b );
    }

    FS_INLINE static mask32v GreaterThan_i32( int32v a, int32v b )
    {
        return _mm256_cmpgt_epi32( a, b );
    }

    FS_INLINE static mask32v LessThan_i32( int32v a, int32v b )
    {
        return _mm256_cmpgt_epi32( b, a );
    }

    // Select

    FS_INLINE static float32v Select_f32( mask32v m, float32v a, float32v b )
    {
        return  _mm256_blendv_ps( b, a, _mm256_castsi256_ps( m ) );
    }

    FS_INLINE static int32v Select_i32( mask32v m, int32v a, int32v b )
    {
        return _mm256_castps_si256( _mm256_blendv_ps( _mm256_castsi256_ps( b ), _mm256_castsi256_ps( a ), _mm256_castsi256_ps( m ) ) );
    }

    // Min, Max

    FS_INLINE static float32v Min_f32( float32v a, float32v b )
    {
        return _mm256_min_ps( a, b );
    }

    FS_INLINE static float32v Max_f32( float32v a, float32v b )
    {
        return _mm256_max_ps( a, b );
    }

    FS_INLINE static int32v Min_i32( int32v a, int32v b )
    {
        return _mm256_min_epi32( a, b );
    }

    FS_INLINE static int32v Max_i32( int32v a, int32v b )
    {
        return _mm256_max_epi32( a, b );
    }

    // Bitwise

    FS_INLINE static float32v BitwiseAnd_f32( float32v a, float32v b )
    {
        return _mm256_and_ps( a, b );
    }

    FS_INLINE static float32v BitwiseOr_f32( float32v a, float32v b )
    {
        return _mm256_or_ps( a, b );
    }

    FS_INLINE static float32v BitwiseXor_f32( float32v a, float32v b )
    {
        return _mm256_xor_ps( a, b );
    }

    FS_INLINE static float32v BitwiseNot_f32( float32v a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        const __m256i neg1 = _mm256_cmpeq_epi32( _mm256_setzero_si256(), _mm256_setzero_si256() );
#else
        const __m256i neg1 = _mm256_set1_epi32( -1 );
#endif
        return _mm256_xor_ps( a, _mm256_castsi256_ps( neg1 ) );
    }

    FS_INLINE static float32v BitwiseAndNot_f32( float32v a, float32v b )
    {
        return _mm256_andnot_ps( b, a );
    }

    FS_INLINE static int32v BitwiseAndNot_i32( int32v a, int32v b )
    {
        return _mm256_andnot_si256( b, a );
    }

    // Abs

    FS_INLINE static float32v Abs_f32( float32v a )
    {
#ifdef FASTSIMD_CONFIG_GENERATE_CONSTANTS
        const __m256i intMax = _mm256_srli_epi32( _mm256_cmpeq_epi32( _mm256_setzero_si256(), _mm256_setzero_si256() ), 1 );
#else
        const __m256i intMax = _mm256_set1_epi32( 0x7FFFFFFF );
#endif
        return _mm256_and_ps( a, _mm256_castsi256_ps( intMax ) );
    }

    FS_INLINE static int32v Abs_i32( int32v a )
    {
        return _mm256_abs_epi32( a );
    }

    // Float math

    FS_INLINE static float32v Sqrt_f32( float32v a )
    {
        return _mm256_sqrt_ps( a );
    }

    FS_INLINE static float32v InvSqrt_f32( float32v a )
    {
        return _mm256_rsqrt_ps( a );
    }

    // Floor, Ceil, Round

    FS_INLINE static float32v Floor_f32( float32v a )
    {
        return _mm256_floor_ps( a );
    }

    FS_INLINE static float32v Ceil_f32( float32v a )
    {
        return _mm256_ceil_ps( a );
    }

    FS_INLINE static float32v Round_f32( float32v a )
    {
        return _mm256_round_ps( a, _MM_FROUND_NINT );
    }

    //Mask

    FS_INLINE static int32v Mask_i32( mask32v m, int32v a )
    {
        return a & m;
    }
};

#if FASTSIMD_COMPILE_AVX
typedef FastSIMD_AVX_T<FastSIMD::Level_AVX>   FastSIMD_AVX;
#endif
#if FASTSIMD_COMPILE_AVX2
typedef FastSIMD_AVX_T<FastSIMD::Level_AVX2>  FastSIMD_AVX2;
#endif
