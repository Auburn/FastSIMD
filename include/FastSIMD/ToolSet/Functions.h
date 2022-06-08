#pragma once

#include "Register.h"
#include "x86/SSE.h"

namespace FS
{
    namespace impl {}
    using namespace impl;

    // Splat value across whole register
    template<std::size_t N, FastSIMD::FeatureSet SIMD = FASTSIMD_DEFAULT_FEATURE_SET, typename T>
    FS_FORCEINLINE Register<T, N, SIMD> Splat( T a )
    {
        return Register<T, N, SIMD>( a );
    }

    // Load values
    template<std::size_t N, FastSIMD::FeatureSet SIMD = FASTSIMD_DEFAULT_FEATURE_SET, typename T>
    FS_FORCEINLINE Register<T, N, SIMD> Load( const T* ptr )
    {
        return Load( TypeWrapper<const T*, N, SIMD>( ptr ) );
    }

    template<typename R>
    FS_FORCEINLINE R Load( const typename R::ElementType* ptr )
    {
        return Load( TypeWrapper<const typename R::ElementType*, R::ElementCount, R::FeatureFlags>( ptr ) );
    }

    namespace impl { template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Load( TypeWrapper<const T*, N, SIMD> ptr )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Load( ptr.AsHalf() ), Load( decltype(ptr)(ptr.value + N / 2).AsHalf() ) };
    } }


    // Store values
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE void Store( typename Register<T, N, SIMD>::ElementType* ptr, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        Store( ptr, a.v0 );
        Store( ptr + N / 2, a.v1 );
    }


    // Convert values
    template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Convert( const Register<T, N, SIMD>& a )
    {
        return Convert( a, TypeDummy<U>{} );
    }

    namespace impl { template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Convert( const Register<T, N, SIMD>& a, TypeDummy<U> )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<U, N, SIMD>{ Convert( a.v0, TypeDummy<U>{} ), Convert( a.v1, TypeDummy<U>{} ) };
    } }


    // Cast values
    template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Cast( const Register<T, N, SIMD>& a )
    {
        return Cast( a, TypeDummy<U>{} );
    }

    namespace impl { template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Cast( const Register<T, N, SIMD>& a, TypeDummy<U> )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<U, N, SIMD>{ Cast( a.v0, TypeDummy<U>{} ), Cast( a.v1, TypeDummy<U>{} ) };
    } }


    // Abs value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Abs( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Abs( a.v0 ), Abs( a.v1 ) };
    }

    
    // Round value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Round( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Round( a.v0 ), Round( a.v1 ) };
    }


    // Floor value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Floor( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Floor( a.v0 ), Floor( a.v1 ) };
    }


    // Ceil value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Ceil( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Ceil( a.v0 ), Ceil( a.v1 ) };
    }


    // Min of 2 elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Min( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Min( a.v0, b.v0 ), Min( a.v1, b.v1 ) };
    }


    // Max of 2 elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Max( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Max( a.v0, b.v0 ), Max( a.v1, b.v1 ) };
    }


    // Select element from 2 registers using Mask
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Select( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& ifTrue, const Register<T, N, SIMD>& ifFalse )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Select( mask.v0, ifTrue.v0, ifFalse.v0 ), Select( mask.v1, ifTrue.v1, ifFalse.v1 ) };
    }
    
}
