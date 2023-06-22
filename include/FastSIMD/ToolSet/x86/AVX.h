#pragma once
#include <immintrin.h>

#if FASTSIMD_MAX_FEATURE_VALUE() >= FASTSIMD_FEATURE_VALUE( AVX512 )
#include "AVX512.h"
#endif

#include "256/f32x8.h"
#include "256/i32x8.h"
#include "256/m32x8.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Convert( const f32<8, SIMD>& a, TypeDummy<int32_t> )
    {
        return _mm256_cvtps_epi32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Convert( const i32<8, SIMD>& a, TypeDummy<float> )
    {
        return _mm256_cvtepi32_ps( a.native );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 8, SIMD>>>
    FS_FORCEINLINE Register<U, 8, SIMD> Cast( const Register<T, 8, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( 
            std::is_same_v<typename Register<T, 8, SIMD>::NativeType, __m256> &&
            std::is_same_v<typename Register<U, 8, SIMD>::NativeType, __m256i> )
        {
            return _mm256_castps_si256( a.GetNative() );
        }
        else if constexpr( 
            std::is_same_v<typename Register<T, 8, SIMD>::NativeType, __m256i> &&
            std::is_same_v<typename Register<U, 8, SIMD>::NativeType, __m256> )
        {
            return _mm256_castsi256_ps( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}
