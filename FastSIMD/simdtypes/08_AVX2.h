#pragma once

#include <intrin.h>

#include "base.h"


struct AVX2_f32x4
{
    FS_VECTOR_TYPE_SET(AVX2_f32x4, __m256);

    AVX2_f32x4& FS_VECTORCALL operator+=(const AVX2_f32x4& rhs)
    {
        *this = _mm256_add_ps(*this, rhs);
        return *this;
    }

    AVX2_f32x4& FS_VECTORCALL operator-=(const AVX2_f32x4& rhs)
    {
        *this = _mm256_sub_ps(*this, rhs);
        return *this;
    }

    AVX2_f32x4& FS_VECTORCALL operator*=(const AVX2_f32x4& rhs)
    {
        *this = _mm256_mul_ps(*this, rhs);
        return *this;
    }

    AVX2_f32x4& FS_VECTORCALL operator/=(const AVX2_f32x4& rhs)
    {
        *this = _mm256_div_ps(*this, rhs);
        return *this;
    }
};

//FS_VECTOR_OPERATORS_FLOAT(AVX2_f32x4)


struct AVX2_i32x4
{
    FS_VECTOR_TYPE_SET(AVX2_i32x4, __m256i);

    AVX2_i32x4& FS_VECTORCALL operator+=(const AVX2_i32x4& rhs)
    {
        *this = _mm256_add_epi32(*this, rhs);
        return *this;
    }

    AVX2_i32x4& FS_VECTORCALL operator-=(const AVX2_i32x4& rhs)
    {
        *this = _mm256_sub_epi32(*this, rhs);
        return *this;
    }

    AVX2_i32x4& FS_VECTORCALL operator*=(const AVX2_i32x4& rhs)
    {
        *this = _mm256_mullo_epi32(*this, rhs);
        return *this;
    }
};

//FS_VECTOR_OPERATORS_FLOAT(AVX2_i32x4)


class FastSIMD_AVX2
{
public:
    typedef AVX2_f32x4 v_f32;
    typedef AVX2_i32x4 v_i32;

    typedef const v_f32& v_f32_arg;
    typedef const v_i32& v_i32_arg;

    static v_f32 FS_VECTORCALL Set1_f32(float a)
    {
        return _mm256_set1_ps(a);
    }

    static v_i32 FS_VECTORCALL Set1_i32(int32_t a)
    {
        return _mm256_set1_epi32(a);
    }

    static void FS_VECTORCALL Store_f32(float* p, v_f32_arg a)
    {
        return _mm256_store_ps(p, a);
    }

    static void FS_VECTORCALL Store_i32(void* p, v_i32_arg a)
    {
        return _mm256_store_si256(static_cast<__m256i*>(p), a);
    }
};