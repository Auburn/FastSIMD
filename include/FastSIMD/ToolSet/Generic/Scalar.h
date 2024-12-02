#pragma once
#include "Scalar/i32x1.h"
#include "Scalar/f32x1.h"
#include "Scalar/mNx1.h"

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Convert( const f32<1, SIMD>& a, TypeDummy<int32_t> )
    {
        return static_cast<std::int32_t>( std::rintf( a.GetNative() ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Convert( const i32<1, SIMD>& a, TypeDummy<float> )
    {
        return static_cast<float>( a.GetNative() );
    }

    template<typename U, typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<U, 1, SIMD> Cast( const Register<T, 1, SIMD>& a, TypeDummy<U> )
    {
        if constexpr( !std::is_same_v<typename Register<T, 1, SIMD>::NativeType, typename Register<U, 1, SIMD>::NativeType> )
        {
            union 
            {
                typename Register<T, 1, SIMD>::NativeType a;
                typename Register<U, 1, SIMD>::NativeType b;
            } u;

            u.a = a.GetNative();

            return u.b;
        }
        else
        {
            return a.GetNative();
        }
    }

    
    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Load( TypeWrapper<const T*, 1, SIMD> ptr )
    {
        return *ptr.value;
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE void Store( T* ptr, const Register<T, 1, SIMD>& a )
    {
        *ptr = a.GetNative();
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE T Extract0( const Register<T, 1, SIMD>& a )
    {
        return a.GetNative();
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Abs( const Register<T, 1, SIMD>& a )
    {
        return std::abs( a.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Round( const Register<T, 1, SIMD>& a )
    {
        return std::rint( a.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Ceil( const Register<T, 1, SIMD>& a )
    {
        return std::ceil( a.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Floor( const Register<T, 1, SIMD>& a )
    {
        return std::floor( a.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Trunc( const Register<T, 1, SIMD>& a )
    {
        return std::trunc( a.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>, typename = EnableIfNotRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Modulus( const Register<T, 1, SIMD>& a, const Register<T, 1, SIMD>& b )
    {
        return std::fmod( a.GetNative(), b.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Min( const Register<T, 1, SIMD>& a, const Register<T, 1, SIMD>& b )
    {
        return std::min( a.GetNative(), b.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Max( const Register<T, 1, SIMD>& a, const Register<T, 1, SIMD>& b )
    {
        return std::max( a.GetNative(), b.GetNative() );
    }

    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Select( const typename Register<T, 1, SIMD>::MaskTypeArg& mask, const Register<T, 1, SIMD>& ifTrue, const Register<T, 1, SIMD>& ifFalse )
    {
        return mask.GetNative() ? ifTrue : ifFalse;
    }

    
    
    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> BitwiseAndNot( const Register<T, 1, SIMD>& a, const Register<T, 1, SIMD>& b )
    {
        return a & ~b;
    }
    
    template<typename T, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<Register<T, 1, SIMD>>>
    FS_FORCEINLINE Register<T, 1, SIMD> Masked( const typename Register<T, 1, SIMD>::MaskTypeArg& mask, const Register<T, 1, SIMD>& a )
    {
        return mask.native ? a : 0;
    }
}
