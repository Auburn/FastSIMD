#pragma once

#if FASTSIMD_MAX_FEATURE_VALUE() >= FASTSIMD_FEATURE_VALUE( WASM )
#include <cmath>
#include <wasm_simd128.h>

#include "128/f32x4.h"
#include "128/i32x4.h"
#include "128/m32x4.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Convert( const f32<4, SIMD>& a, TypeDummy<int32_t> )
    {
        if constexpr( FastSIMD::IsRelaxed() )
        {
            return wasm_i32x4_relaxed_trunc_f32x4( Round( a ).native );
        }
        else
        {
            return wasm_i32x4_trunc_sat_f32x4( Round( a ).native );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Convert( const i32<4, SIMD>& a, TypeDummy<float> )
    {
        return wasm_f32x4_convert_i32x4( a.native );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 4, SIMD>>>
    FS_FORCEINLINE Register<U, 4, SIMD> Cast( const Register<T, 4, SIMD>& a, TypeDummy<U> )
    {
        if constexpr(
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, __f32x4> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, v128_t> )
        {
            return static_cast<__f32x4>( a.GetNative() );
        }
        else if constexpr(
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, v128_t> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, __f32x4> )
        {
            return static_cast<v128_t>( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}
#endif
