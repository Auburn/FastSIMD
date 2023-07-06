#pragma once
#include <immintrin.h>

#include "512/f32x16.h"
#include "512/i32x16.h"
#include "512/mNx16.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Convert( const f32<16, SIMD>& a, TypeDummy<int32_t> )
    {
        return _mm512_cvtps_epi32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Convert( const i32<16, SIMD>& a, TypeDummy<float> )
    {
        return _mm512_cvtepi32_ps( a.native );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 16, SIMD>>>
    FS_FORCEINLINE Register<U, 16, SIMD> Cast( const Register<T, 16, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( 
            std::is_same_v<typename Register<T, 16, SIMD>::NativeType, __m512> &&
            std::is_same_v<typename Register<U, 16, SIMD>::NativeType, __m512i> )
        {
            return _mm512_castps_si512( a.GetNative() );
        }
        else if constexpr( 
            std::is_same_v<typename Register<T, 16, SIMD>::NativeType, __m512i> &&
            std::is_same_v<typename Register<U, 16, SIMD>::NativeType, __m512> )
        {
            return _mm512_castsi512_ps( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}
