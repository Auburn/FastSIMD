#pragma once
#include <arm_neon.h>

#include "128/f32x4.h"
#include "128/i32x4.h"
#include "128/m32x4.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Convert( const f32<4, SIMD>& a, TypeDummy<int32_t> )
    {
        return vcvtq_s32_f32( Round( a ).native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Convert( const i32<4, SIMD>& a, TypeDummy<float> )
    {
        return vcvtq_f32_s32( a.native );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 4, SIMD>>>
    FS_FORCEINLINE Register<U, 4, SIMD> Cast( const Register<T, 4, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( 
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, float32x4_t> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, int32x4_t> )
        {
            return vreinterpretq_s32_f32( a.GetNative() );
        }
        else if constexpr( 
            std::is_same_v<typename Register<T, 4, SIMD>::NativeType, int32x4_t> &&
            std::is_same_v<typename Register<U, 4, SIMD>::NativeType, float32x4_t> )
        {
            return vreinterpretq_f32_s32( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}
