#pragma once

#include "Register.h"
#include "x86/SSE.h"

namespace FS
{
    // Load values
    template<typename R>
    FS_FORCEINLINE R Load( const typename R::ElementType* ptr )
    {
        return Load( TypeWrapper<const typename R::ElementType*, R::ElementCount, R::SimdFlags>( ptr ) );
    }

    template<std::size_t N, FastSIMD::FeatureSet SIMD = FASTSIMD_DEFAULT_FEATURE_SET, typename T>
    FS_FORCEINLINE Register<T, N, SIMD> Load( const T* ptr )
    {
        return Load( TypeWrapper<const T*, N, SIMD>( ptr ) );
    }
    
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Load( TypeWrapper<const T*, N, SIMD> ptr )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Load( ptr.AsHalf() ), Load( decltype(ptr)(ptr.value + N / 2).AsHalf() ) };
    }

    // Store values
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE void Store( typename Register<T, N, SIMD>::ElementType* ptr, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        Store( ptr, a.v0 );
        Store( ptr + N / 2, a.v1 );
    }

    // Abs value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Abs( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Abs( a.v0 ), Abs( a.v1 ) };
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

    // Select element from 2 vectors using Mask
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Select( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& ifTrue, const Register<T, N, SIMD>& ifFalse )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Select( mask.v0, ifTrue.v0, ifFalse.v0 ), Select( mask.v1, ifTrue.v1, ifFalse.v1 ) };
    }
    
}

//#include <array>
//
//int Test()
//{
//    std::array<int, 8> data;
//    std::fill( data.begin(), data.end(), 3 );
//
//    auto a = FS::Load<FS::i32<8>>( data.data() );
//    auto b = FS::i32<8>( 1 );
//
//    a += b;
//    a = b - b;
//    a += b;
//
//    a = FS::Min( a, b );
//    a = FS::Abs( a );
//
//    FS::m32<8> m = (a == b);
//
//    a = FS::Select( m, b, a );
//
//    /*FS::f32<4> c, d;
//    c &= d;
//    
//    d = FS::Select( a == b, c, d );*/
//
//    return memcmp(&a, &b, sizeof(a));
//}