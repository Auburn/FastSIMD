#pragma once
#include "Scalar/i32x1.h"
#include "Scalar/f32x1.h"
#include "Scalar/mNx1.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Convert( const f32<1, SIMD>& a, TypeDummy<int32_t> )
    {
        return static_cast<std::int32_t>( std::lrintf( a.GetNative() ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Convert( const i32<1, SIMD>& a, TypeDummy<float> )
    {
        return static_cast<float>( a.GetNative() );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, 1, SIMD> Cast( const Register<T, 1, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( !std::is_same_v<typename Register<T, 1, SIMD>::NativeType, typename Register<U, 1, SIMD>::NativeType> )
        {
            return ScalarCast<U>( a.GetNative() );
        }
        else
        {
            return a.GetNative();
        }
    }
}