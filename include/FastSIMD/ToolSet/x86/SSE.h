#pragma once
#include <nmmintrin.h>

#if FASTSIMD_MAX_FEATURE_VALUE() >= FASTSIMD_FEATURE_VALUE( AVX )
#include "AVX.h"
#endif

#include "128/f32x4.h"
#include "128/i32x4.h"
#include "128/m32x4.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Convert( const f32<4, SIMD>& a, TypeDummy<int32_t> )
    {
        return _mm_cvtps_epi32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Convert( const i32<4, SIMD>& a, TypeDummy<float> )
    {
        return _mm_cvtepi32_ps( a.native );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 4, SIMD>>>
    FS_FORCEINLINE Register<U, 4, SIMD> Cast( const Register<T, 4, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( 
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, __m128> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, __m128i> )
        {
            return _mm_castps_si128( a.GetNative() );
        }
        else if constexpr( 
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, __m128i> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, __m128> )
        {
            return _mm_castsi128_ps( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}
