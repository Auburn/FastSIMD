#pragma once

#include <intrin.h>

#include "base.h"

struct SSE2_f32x4
{
    FS_VECTOR_TYPE_SET(SSE2_f32x4, __m128);
    
    FS_INLINE static __m128 CONST(float I0, float I1, float I2, float I3)
    {
        static const union
        {
            float i[4];
            __m128 v;
        } u = { { I0, I1, I2, I3 } };
        return u.v;
    }

    FS_INLINE SSE2_f32x4& operator+=(const SSE2_f32x4& rhs)
    {
        *this = _mm_add_ps(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator-=(const SSE2_f32x4& rhs)
    {
        *this = _mm_sub_ps(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator*=(const SSE2_f32x4& rhs)
    {
        *this = _mm_mul_ps(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_f32x4& operator/=(const SSE2_f32x4& rhs)
    {
        *this = _mm_div_ps(*this, rhs);
        return *this;
    }
};

FS_VECTOR_OPERATORS_FLOAT(SSE2_f32x4)


struct SSE2_i32x4
{
    FS_VECTOR_TYPE_SET(SSE2_i32x4, __m128i);

    template <int32_t I0, int32_t I1, int32_t I2, int32_t I3>
    FS_INLINE static __m128i CONST()
    {
        static const union
        {
            int32_t i[4];
            __m128i v;
        } u = { { I0, I1, I2, I3 } };
        return u.v;
    }

    FS_INLINE SSE2_i32x4& operator+=(const SSE2_i32x4& rhs)
    {
        *this = _mm_add_epi32(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator-=(const SSE2_i32x4& rhs)
    {
        *this = _mm_sub_epi32(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator*=(const SSE2_i32x4& rhs)
    {     
        __m128 tmp1 = _mm_castsi128_ps(_mm_mul_epu32(*this, rhs)); /* mul 2,0*/
        __m128 tmp2 = _mm_castsi128_ps(_mm_mul_epu32(_mm_srli_si128(*this, 4), _mm_srli_si128(rhs, 4))); /* mul 3,1 */
        *this = _mm_shuffle_epi32(_mm_castps_si128(_mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(2, 0, 2, 0))), _MM_SHUFFLE(3, 1, 2, 0));
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator&=(const SSE2_i32x4& rhs)
    {
        *this = _mm_and_si128(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator|=(const SSE2_i32x4& rhs)
    {
        *this = _mm_or_si128(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator^=(const SSE2_i32x4& rhs)
    {
        *this = _mm_xor_si128(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator>>=(int32_t rhs)
    {
        *this = _mm_srai_epi32(*this, rhs);
        return *this;
    }

    FS_INLINE SSE2_i32x4& operator<<=(int32_t rhs)
    {
        *this = _mm_slli_epi32(*this, rhs);
        return *this;
    }
};

FS_VECTOR_OPERATORS_INT(SSE2_i32x4)


class FastSIMD_SSE2
{
public:
    static const FastSIMD::Level SIMD_Level = FastSIMD::Level_SSE2;

    typedef SSE2_f32x4 float32_v;
    typedef SSE2_i32x4 int32_v;

    typedef const float32_v& float32_v_arg;
    typedef const int32_v&   int32_v_arg;

    FS_INLINE static float32_v SetAll_f32(float a)
    {
        return _mm_set1_ps(a);
    }

    FS_INLINE static int32_v SetAll_i32(int32_t a)
    {
        return _mm_set1_epi32(a);
    }

    FS_INLINE static void Store_f32(float* p, float32_v_arg a)
    {
        return _mm_store_ps(p, a);
    }

    FS_INLINE static void Store_i32(void* p, int32_v_arg a)
    {
        return _mm_store_si128(static_cast<__m128i*>(p), a);
    }
};