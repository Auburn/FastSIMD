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
        // no direct alternative in WASM, cannot be vectorized
        // https://emscripten.org/docs/porting/simd.html
        // TODO: optimize
        union {
            int32_t x[4];
            v128_t m;
        } u;
        for(int i = 0; i < 4; ++i)
        {
            int x = lrint(a.native[i]);
            if (x != 0 || fabs(a.native[i]) < 2.0)
                u.x[i] = x;
            else
                u.x[i] = (int)0x80000000;
        }
        return u.m;
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
