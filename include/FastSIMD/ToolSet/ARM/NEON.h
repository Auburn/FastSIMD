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

    namespace impl
    {
        // Native casts go through uint32x4_t and pick the reinterpret by element type, not native type
        // With the MSVC ARM64 ABI every NEON vector type is the same __n128, so native types can't be told apart
        template<typename T, typename V>
        FS_FORCEINLINE uint32x4_t NeonCastToU32( V a )
        {
            if constexpr( std::is_same_v<T, float> )
            {
                return vreinterpretq_u32_f32( a );
            }
            else if constexpr( std::is_same_v<T, std::int32_t> )
            {
                return vreinterpretq_u32_s32( a );
            }
            else
            {
                static_assert( std::is_same_v<T, Mask<32, true>> || std::is_same_v<T, Mask<32, false>>, "FastSIMD: FS::Cast not supported with provided types" );
                return a;
            }
        }

        template<typename U>
        FS_FORCEINLINE auto NeonCastFromU32( uint32x4_t a )
        {
            if constexpr( std::is_same_v<U, float> )
            {
                return vreinterpretq_f32_u32( a );
            }
            else if constexpr( std::is_same_v<U, std::int32_t> )
            {
                return vreinterpretq_s32_u32( a );
            }
            else
            {
                static_assert( std::is_same_v<U, Mask<32, true>> || std::is_same_v<U, Mask<32, false>>, "FastSIMD: FS::Cast not supported with provided types" );
                return a;
            }
        }
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 4, SIMD>>>
    FS_FORCEINLINE Register<U, 4, SIMD> Cast( const Register<T, 4, SIMD>& a, TypeDummy<U> )
    {
        return impl::NeonCastFromU32<U>( impl::NeonCastToU32<T>( a.GetNative() ) );
    }
}
