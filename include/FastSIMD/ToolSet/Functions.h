#pragma once

#include <algorithm>

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
        return Register<T, N, SIMD>{ Load( ptr.AsHalf() ), Load( ptr.AsHalf( N / 2 ) ) };
    } }


    // Store values
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE void Store( typename Register<T, N, SIMD>::ElementType* ptr, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        Store( ptr, a.v0 );
        Store( ptr + N / 2, a.v1 );
    }
    
    // Load constant set of values into register
    template<typename T, FastSIMD::FeatureSet SIMD = FASTSIMD_DEFAULT_FEATURE_SET, std::int64_t... VALUEs>
    FS_FORCEINLINE Register<T, sizeof...( VALUEs ), SIMD> Constant( std::integer_sequence<std::int64_t, VALUEs...> )
    {
        alignas( std::alignment_of_v<Register<T, sizeof...( VALUEs ), SIMD>> ) const T constArray[] = { T( VALUEs )... };

        return Load<sizeof...( VALUEs ), SIMD>( constArray );
    }
    
    // Extract first element
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE T Extract0( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Extract0( a.v0 );
    }

    // Incremented elements in a register { 0, 1, 2, ..., N - 1 }
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD = FASTSIMD_DEFAULT_FEATURE_SET>
    FS_FORCEINLINE Register<T, N, SIMD> Incremented()
    {
        return Constant<T, SIMD>( std::make_integer_sequence<std::int64_t, N>{} );
    }

    template<typename R>
    FS_FORCEINLINE R Incremented()
    {
        return Constant<typename R::ElementType, R::FeatureFlags>( std::make_integer_sequence<std::int64_t, R::ElementCount>{} );
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

    // Increment value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Increment( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Increment( a.v0 ), Increment( a.v1 ) };
    }
    
    // FMulAdd (a * b) + c
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> FMulAdd( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b, const Register<T, N, SIMD>& c )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return (a * b) + c;
        }
        else
        {        
            return Register<T, N, SIMD>{ FMulAdd( a.v0, b.v0, c.v0 ), FMulAdd( a.v1, b.v1, c.v1 ) };
        }
    }
    
    // Bitwise AndNot
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> BitwiseAndNot( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ BitwiseAndNot( a.v0, b.v0 ), BitwiseAndNot( a.v1, b.v1 ) };
    }
    
    // Mask elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Masked( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: Function not supported with provided types" );
        return Register<T, N, SIMD>{ Masked( mask.v0, a.v0 ), Masked( mask.v1, a.v1 ) };
    }
    
    // Masked Increment
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> MaskedIncrement( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a - Masked( mask, Register<T, N, SIMD>( -1 ) );
        }
        else
        {   
            return Register<T, N, SIMD>{ MaskedIncrement( mask.v0, a.v0 ), MaskedIncrement( mask.v1, a.v1 ) };
        }
    }
    
    // Masked Add
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> MaskedAdd( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a + Masked( mask, b );
        }
        else
        {        
            return Register<T, N, SIMD>{ MaskedAdd( mask.v0, a.v0, b.v0 ), MaskedAdd( mask.v1, a.v1, b.v1 ) };
        }
    }
    
    // Masked Sub
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> MaskedSub( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a - Masked( mask, b );
        }
        else
        {        
            return Register<T, N, SIMD>{ MaskedSub( mask.v0, a.v0, b.v0 ), MaskedSub( mask.v1, a.v1, b.v1 ) };
        }
    }
    
    template<std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<float, N, SIMD> Cos( const Register<float, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<float, N, SIMD>> )
        {
            using RegFloat = Register<float, N, SIMD>;
            using RegInt = Register<std::int32_t, N, SIMD>;

            RegFloat value = Abs( a );
            value -= Floor( value * RegFloat( 0.1591549f ) ) * RegFloat( 6.283185f );

            auto geHalfPi  = value >= RegFloat( 1.570796f );
            auto geHalfPi2 = value >= RegFloat( 3.141593f );
            auto geHalfPi3 = value >= RegFloat( 4.7123889f );

            RegFloat cosAngle;
            cosAngle = value ^ Masked( geHalfPi, value ^ ( RegFloat( 3.141593f ) - value ) );
            cosAngle = cosAngle ^ Masked( geHalfPi2, Cast<float>( RegInt( 0x80000000 ) ) );
            cosAngle = cosAngle ^ Masked( geHalfPi3, cosAngle ^ ( RegFloat( 6.283185f ) - value ) );

            cosAngle *= cosAngle;

            cosAngle = FMulAdd( cosAngle, FMulAdd( cosAngle, RegFloat( 0.03679168f ), RegFloat( -0.49558072f ) ), RegFloat( 0.99940307f ) );

            return cosAngle ^ Masked( BitwiseAndNot( geHalfPi, geHalfPi3 ), Cast<float>( RegInt( 0x80000000 ) ) );
        }
        else
        {   
            return Register<float, N, SIMD>{ Cos( a.v0 ), Cos( a.v1 ) };
        }
    }
    
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Sin( const Register<T, N, SIMD>& a )
    {
        return Cos( Register<T, N, SIMD>( (T)1.57079632679 ) - a );
    }
}
