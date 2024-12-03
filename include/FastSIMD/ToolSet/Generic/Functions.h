#pragma once

#include <algorithm>
#include <limits>

#include "Register.h"

#define FS_BIND_INTRINSIC( FUNC ) []( auto... ARGS ){ return FUNC( ARGS... ); }

namespace FS
{
    namespace impl {}
    using namespace impl;

    // Splat value across whole register
    template<std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault(), typename T>
    FS_FORCEINLINE Register<T, N, SIMD> Splat( T a )
    {
        return Register<T, N, SIMD>( a );
    }


    // impl
    namespace impl { template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Load( TypeWrapper<const T*, N, SIMD> ptr )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Load not supported with provided types" );
        return Register<T, N, SIMD>{ Load( ptr.AsHalf() ), Load( ptr.AsHalf( N / 2 ) ) };
    } }

    // Load values
    template<std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault(), typename T>
    FS_FORCEINLINE Register<T, N, SIMD> Load( const T* ptr )
    {
        return Load( TypeWrapper<const T*, N, SIMD>( ptr ) );
    }

    template<typename R>
    FS_FORCEINLINE R Load( const typename R::ElementType* ptr )
    {
        return Load( TypeWrapper<const typename R::ElementType*, R::ElementCount, R::FeatureFlags>( ptr ) );
    }


    // Store values
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE void Store( typename Register<T, N, SIMD>::ElementType* ptr, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Store not supported with provided types" );
        Store( ptr, a.v0 );
        Store( ptr + N / 2, a.v1 );
    }
    
    template<std::size_t S, bool F, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE BitStorage<N> BitMask( const Register<Mask<S, F>, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<Mask<S, F>, N, SIMD>>, "FastSIMD: FS::BitMask not supported with provided types" );
        return static_cast<BitStorage<N>>( BitMask( a.v0 ) ) | static_cast<BitStorage<N>>( BitMask( a.v1 ) << ( N / 2 ) );
    }
    
    // Load constant set of values into register
    template<typename T, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault(), typename... Ts>
    FS_FORCEINLINE Register<T, sizeof...( Ts ) + 1, SIMD> Constant( T value, Ts... values )
    {
        alignas( std::alignment_of_v<Register<T, sizeof...( Ts ) + 1, SIMD>> ) const T constArray[] = { value, T( values )... };

        return Load<sizeof...( Ts ) + 1, SIMD>( constArray );
    }
    
    template<typename T, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault(), std::int64_t... VALUEs>
    FS_FORCEINLINE Register<T, sizeof...( VALUEs ), SIMD> Constant( std::integer_sequence<std::int64_t, VALUEs...> )
    {
        alignas( std::alignment_of_v<Register<T, sizeof...( VALUEs ), SIMD>> ) const T constArray[] = { T( VALUEs )... };

        return Load<sizeof...( VALUEs ), SIMD>( constArray );
    }
    
    // Extract first element
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE T Extract0( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Extract0 not supported with provided types" );
        return Extract0( a.v0 );
    }

    // Incremented elements in a register { 0, 1, 2, ..., N - 1 }
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    FS_FORCEINLINE Register<T, N, SIMD> LoadIncremented()
    {
        return Constant<T, SIMD>( std::make_integer_sequence<std::int64_t, N>{} );
    }

    template<typename R>
    FS_FORCEINLINE R LoadIncremented()
    {
        return Constant<typename R::ElementType, R::FeatureFlags>( std::make_integer_sequence<std::int64_t, R::ElementCount>{} );
    }

    // impl
    namespace impl { template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Convert( const Register<T, N, SIMD>& a, TypeDummy<U> )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS:Convert not supported with provided types" );
        return Register<U, N, SIMD>{ Convert( a.v0, TypeDummy<U>{} ), Convert( a.v1, TypeDummy<U>{} ) };
    } }

    // Convert values
    template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Convert( const Register<T, N, SIMD>& a )
    {
        return Convert( a, TypeDummy<U>{} );
    }


    // impl
    namespace impl { template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Cast( const Register<T, N, SIMD>& a, TypeDummy<U> )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS:Cast not supported with provided types" );
        return Register<U, N, SIMD>{ Cast( a.v0, TypeDummy<U>{} ), Cast( a.v1, TypeDummy<U>{} ) };
    } }

    // Cast values
    template<typename U, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<U, N, SIMD> Cast( const Register<T, N, SIMD>& a )
    {
        return Cast( a, TypeDummy<U>{} );
    }


    // Abs value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Abs( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS:Abs not supported with provided types" );
        return Register<T, N, SIMD>{ Abs( a.v0 ), Abs( a.v1 ) };
    }

    
    // Round value, banker's rounding
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Round( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Round not supported with provided types" );
        return Register<T, N, SIMD>{ Round( a.v0 ), Round( a.v1 ) };
    }


    // Floor value, round towards -infinity
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Floor( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Floor not supported with provided types" );
        return Register<T, N, SIMD>{ Floor( a.v0 ), Floor( a.v1 ) };
    }


    // Ceil value, round towards +infinity
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Ceil( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Ceil not supported with provided types" );
        return Register<T, N, SIMD>{ Ceil( a.v0 ), Ceil( a.v1 ) };
    }

    // Truncate value, round towards 0
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Trunc( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Trunc not supported with provided types" );
        return Register<T, N, SIMD>{ Trunc( a.v0 ), Trunc( a.v1 ) };
    }


    // Min of 2 elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Min( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Min not supported with provided types" );
        return Register<T, N, SIMD>{ Min( a.v0, b.v0 ), Min( a.v1, b.v1 ) };
    }


    // Max of 2 elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Max( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Max not supported with provided types" );
        return Register<T, N, SIMD>{ Max( a.v0, b.v0 ), Max( a.v1, b.v1 ) };
    }


    // Select element from 2 registers using Mask
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Select( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& ifTrue, const Register<T, N, SIMD>& ifFalse )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Select not supported with provided types" );
        return Register<T, N, SIMD>{ Select( mask.v0, ifTrue.v0, ifFalse.v0 ), Select( mask.v1, ifTrue.v1, ifFalse.v1 ) };
    }

    // Select element from 2 registers using most significant bit from int or float
    template<typename T, typename U, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> SelectHighBit( const Register<U, N, SIMD>& mask, const Register<T, N, SIMD>& ifTrue, const Register<T, N, SIMD>& ifFalse )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            if constexpr( std::is_same_v<typename Register<U, N, SIMD>::NativeType, typename Register<U, N, SIMD>::MaskType::NativeType> )
        {
                return Select( FS::Cast<typename Register<U, N, SIMD>::MaskType::ElementType>( FS::Cast<std::int32_t>( mask ) >> 31 ), ifTrue, ifFalse );                
            }
            else
            {
                return Select( FS::Cast<std::int32_t>( mask ) < Register<std::int32_t, N, SIMD>( 0 ), ifTrue, ifFalse );
            }
        }
        else
        {
            return Register<T, N, SIMD>{ SelectHighBit( mask.v0, ifTrue.v0, ifFalse.v0 ), SelectHighBit( mask.v1, ifTrue.v1, ifFalse.v1 ) };
        }
    }

    // Increment value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Increment( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a - Register<T, N, SIMD>( -1 );
        }
        else
        {
            return Register<T, N, SIMD>{ Increment( a.v0 ), Increment( a.v1 ) };
        }
    }

    // Decrement value
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Decrement( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a + Register<T, N, SIMD>( -1 );
        }
        else
        {
            return Register<T, N, SIMD>{ Decrement( a.v0 ), Decrement( a.v1 ) };
        }
    }
    
    // FMulAdd: (a * b) + c
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
    
    // FMulSub: (a * b) - c
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> FMulSub( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b, const Register<T, N, SIMD>& c )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return (a * b) - c;
        }
        else
        {        
            return Register<T, N, SIMD>{ FMulSub( a.v0, b.v0, c.v0 ), FMulSub( a.v1, b.v1, c.v1 ) };
        }
    }
    
    // FNMulAdd: -(a * b) + c
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> FNMulAdd( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b, const Register<T, N, SIMD>& c )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return c - (a * b);
        }
        else
        {        
            return Register<T, N, SIMD>{ FNMulAdd( a.v0, b.v0, c.v0 ), FNMulAdd( a.v1, b.v1, c.v1 ) };
        }
    }
    
    // FNMulSub: -(a * b) - c
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> FNMulSub( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b, const Register<T, N, SIMD>& c )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return -c - (a * b);
        }
        else
        {        
            return Register<T, N, SIMD>{ FNMulSub( a.v0, b.v0, c.v0 ), FNMulSub( a.v1, b.v1, c.v1 ) };
        }
    }
    
    // Bitwise AndNot
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> BitwiseAndNot( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a & (~b);
        }
        else
        {
            return Register<T, N, SIMD>{ BitwiseAndNot( a.v0, b.v0 ), BitwiseAndNot( a.v1, b.v1 ) };
        }
    }    

    // BitShift Right Zero Extend
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> BitShiftRightZeroExtend( const Register<T, N, SIMD>& a, int b )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::BitShiftRightZeroExtend not supported with provided types" );
        return Register<T, N, SIMD>{ BitShiftRightZeroExtend( a.v0, b ), BitShiftRightZeroExtend( a.v1, b ) };
    }
    
    // Mask elements
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Masked( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Masked not supported with provided types" );
        return Register<T, N, SIMD>{ Masked( mask.v0, a.v0 ), Masked( mask.v1, a.v1 ) };
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> InvMasked( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a )
    {        
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Masked( ~mask, a );
        }
        else
        {   
            return Register<T, N, SIMD>{ InvMasked( mask.v0, a.v0 ), InvMasked( mask.v1, a.v1 ) };
        }
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
    
    // Masked Decrement
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> MaskedDecrement( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a + Masked( mask, Register<T, N, SIMD>( -1 ) );
        }
        else
        {   
            return Register<T, N, SIMD>{ MaskedDecrement( mask.v0, a.v0 ), MaskedDecrement( mask.v1, a.v1 ) };
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

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> InvMaskedAdd( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a + InvMasked( mask, b );
        }
        else
        {        
            return Register<T, N, SIMD>{ InvMaskedAdd( mask.v0, a.v0, b.v0 ), InvMaskedAdd( mask.v1, a.v1, b.v1 ) };
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
    
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> InvMaskedSub( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return a - InvMasked( mask, b );
        }
        else
        {        
            return Register<T, N, SIMD>{ InvMaskedSub( mask.v0, a.v0, b.v0 ), InvMaskedSub( mask.v1, a.v1, b.v1 ) };
        }
    }
    
    // Masked Mul
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> MaskedMul( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Select( mask, a * b, a );
        }
        else
        {        
            return Register<T, N, SIMD>{ MaskedMul( mask.v0, a.v0, b.v0 ), MaskedMul( mask.v1, a.v1, b.v1 ) };
        }
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> InvMaskedMul( const typename Register<T, N, SIMD>::MaskTypeArg& mask, const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Select( mask, a, a * b );
        }
        else
        {        
            return Register<T, N, SIMD>{ InvMaskedMul( mask.v0, a.v0, b.v0 ), InvMaskedMul( mask.v1, a.v1, b.v1 ) };
        }
    }
    
    // Extract sign bit (high bit)
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> SignBit( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            Register<T, N, SIMD> signBit;
            if constexpr( std::is_floating_point_v<T> )
            {
                signBit = Register<T, N, SIMD>( (T)-0.0 );
            }
            else
            {
                static_assert( std::is_signed_v<T>, "No signed bit in unsigned type" );
                signBit = Register<T, N, SIMD>( std::numeric_limits<T>::min() );
            }

            return signBit & a;
        }
        else
        {        
            return Register<T, N, SIMD>{ SignBit( a.v0 ), SignBit( a.v1 ) };
        }
    }

    // Modulus: a % b
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Modulus( const Register<T, N, SIMD>& a, const Register<T, N, SIMD>& b )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            auto ab = a / b;
            return (ab - Trunc( ab )) * b;
        }
        else
        {
            return Register<T, N, SIMD>{ Modulus( a.v0, b.v0 ), Modulus( a.v1, b.v1 ) };
        }
    }

    // Reciprocal: 1 / a
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Reciprocal( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Register<T, N, SIMD>( 1 ) / a;
        }
        else
        {
            return Register<T, N, SIMD>{ Reciprocal( a.v0 ), Reciprocal( a.v1 ) };
        }
    }
    
    // Inv Sqrt: 1 / sqrt( a )
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> InvSqrt( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Register<T, N, SIMD>( 1 ) / Sqrt( a );
        }
        else
        {
            return Register<T, N, SIMD>{ InvSqrt( a.v0 ), InvSqrt( a.v1 ) };
        }   
    }

    // Sqrt
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Sqrt( const Register<T, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Sqrt not supported with provided types" );
        return Register<T, N, SIMD>{ Sqrt( a.v0 ), Sqrt( a.v1 ) };
    }

    template<std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<float, N, SIMD> Cos( const Register<float, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<float, N, SIMD>> )
        {
            using RegisterF = Register<float, N, SIMD>;
            using RegisterI = Register<std::int32_t, N, SIMD>;

            RegisterF value = Abs( a );
            value -= Floor( value * RegisterF( 0.1591549f ) ) * RegisterF( 6.283185f );

            auto geHalfPi  = value >= RegisterF( 1.570796f );
            auto geHalfPi2 = value >= RegisterF( 3.141593f );
            auto geHalfPi3 = value >= RegisterF( 4.7123889f );

            RegisterF cosAngle;
            cosAngle = value ^ Masked( geHalfPi, value ^ ( RegisterF( 3.141593f ) - value ) );
            cosAngle = cosAngle ^ Masked( geHalfPi2, Cast<float>( RegisterI( 0x80000000 ) ) );
            cosAngle = cosAngle ^ Masked( geHalfPi3, cosAngle ^ ( RegisterF( 6.283185f ) - value ) );

            cosAngle *= cosAngle;

            cosAngle = FMulAdd( cosAngle, FMulAdd( cosAngle, RegisterF( 0.03679168f ), RegisterF( -0.49558072f ) ), RegisterF( 0.99940307f ) );

            return cosAngle ^ Masked( BitwiseAndNot( geHalfPi, geHalfPi3 ), Cast<float>( RegisterI( 0x80000000 ) ) );
        }
        else
        {   
            return Register<float, N, SIMD>{ Cos( a.v0 ), Cos( a.v1 ) };
        }
    }
    
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Sin( const Register<T, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return Cos( Register<T, N, SIMD>( (T)1.57079632679 ) - a );
        }
        else
        {
            return Register<T, N, SIMD>{ Sin( a.v0 ), Sin( a.v1 ) };
        }
    }
    
    template<std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<float, N, SIMD> Exp( const Register<float, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<float, N, SIMD>> )
        {
            using RegisterF = Register<float, N, SIMD>;
            using RegisterI = Register<std::int32_t, N, SIMD>;

            RegisterF value = a;
            value = Min( value, RegisterF( 88.3762626647949f ) );
            value = Max( value, RegisterF( -88.3762626647949f ) );

            /* express exp(x) as exp(g + n*log(2)) */
            RegisterF fx = value * RegisterF( 1.44269504088896341f );
            fx += RegisterF( 0.5f );

            RegisterF flr = Floor( fx );  
            fx = MaskedDecrement( flr > fx, flr );

            value -= fx * RegisterF( 0.693359375f );
            value -= fx * RegisterF( -2.12194440e-4f );

            RegisterF y( 1.9875691500E-4f );
            y *= value;
            y += RegisterF( 1.3981999507E-3f );
            y *= value;
            y += RegisterF( 8.3334519073E-3f );
            y *= value;
            y += RegisterF( 4.1665795894E-2f );
            y *= value;
            y += RegisterF( 1.6666665459E-1f );
            y *= value;
            y += RegisterF( 5.0000001201E-1f );
            y *= value * value;
            y += value + RegisterF( 1 );        

            /* build 2^n */
            RegisterI i = Convert<std::int32_t>( fx );

            i += RegisterI( 0x7f );
            i <<= 23;
            RegisterF pow2n = Cast<float>( i );
        
            return y * pow2n;
        }
        else
        {   
            return Register<float, N, SIMD>{ Exp( a.v0 ), Exp( a.v1 ) };
        }
    }
    
    template<std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<float, N, SIMD> Log( const Register<float, N, SIMD>& a )
    {
        if constexpr( IsNativeV<Register<float, N, SIMD>> )
        {
            using RegisterF = Register<float, N, SIMD>;
            using RegisterI = Register<std::int32_t, N, SIMD>;
                
            auto validMask = a > RegisterF( 0 );

            RegisterF value = Max( a, Cast<float>( RegisterI( 0x00800000 ) ) );  /* cut off denormalized stuff */
            
            RegisterI i = BitShiftRightZeroExtend( Cast<std::int32_t>( value ), 23 );

            /* keep only the fractional part */
            value &= Cast<float>( RegisterI( ~0x7f800000 ) );
            value |= RegisterF( 0.5f );
            
            i -= RegisterI( 0x7f );
            RegisterF e = Convert<float>( i );

            e += RegisterF( 1 );

            auto mask = value < RegisterF( 0.707106781186547524f );
            value = MaskedAdd( mask, value, value );
            value -= RegisterF( 1 );
            e = MaskedDecrement( mask, e );

            RegisterF y = RegisterF( 7.0376836292E-2f );
            y *= value;
            y += RegisterF( -1.1514610310E-1f );
            y *= value;
            y += RegisterF( 1.1676998740E-1f );
            y *= value;
            y += RegisterF( -1.2420140846E-1f );
            y *= value;
            y += RegisterF( 1.4249322787E-1f );
            y *= value;
            y += RegisterF( -1.6668057665E-1f );
            y *= value;
            y += RegisterF( 2.0000714765E-1f );
            y *= value;
            y += RegisterF( -2.4999993993E-1f );
            y *= value;
            y += RegisterF( 3.3333331174E-1f );
            y *= value;

            RegisterF xx = value * value;
            y *= xx;
            y *= e * RegisterF( -2.12194440e-4f );
            y -= xx * RegisterF( 0.5f );

            value += y;
            value += e * RegisterF( 0.693359375f );

            return Masked( validMask, value );
        }
        else
        {   
            return Register<float, N, SIMD>{ Log( a.v0 ), Log( a.v1 ) };
        }
    }
    
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Pow( const Register<T, N, SIMD>& value, const Register<T, N, SIMD>& pow )
    {
        return Exp( pow * Log( value ) );
    }
    
    // Any Mask
    // returns true if any mask bits are set
    template<std::size_t S, bool F, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<S, F>, N, SIMD>& a )
    {
        static_assert( !IsNativeV<Register<Mask<S, F>, N, SIMD>>, "FastSIMD: FS::AnyMask not supported with provided types" );
        return AnyMask( a.v0 ) || AnyMask( a.v1 );
    }

    namespace impl
    {
        template<bool ZERO, typename T>
        FS_FORCEINLINE constexpr auto NativeExecSplitOrNative( const T& v )
        {
            return v;
        }

        template<bool ZERO, typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE auto NativeExecSplitOrNative( const Register<T, N, SIMD>& v )
        {
            if constexpr( IsNativeV<Register<T, N, SIMD>> )
            {
                return v.GetNative();
            }
            else
            {
                return ZERO ? v.v0 : v.v1;
            }
        }

        template<typename T>
        struct NativeExecIsNative : std::true_type {};

        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        struct NativeExecIsNative<Register<T, N, SIMD>> : std::bool_constant<IsNativeV<Register<T, N, SIMD>>> {};
    }

    template<typename R, typename FUNC, typename... ARGS>
    FS_FORCEINLINE R NativeExec( FUNC f, const ARGS&... args )
    {
        using T = typename R::ElementType;
        constexpr std::size_t N = R::ElementCount;
        constexpr FastSIMD::FeatureSet SIMD = R::FeatureFlags;

        //if constexpr( IsNativeV<Register<T, N, SIMD>> )
        if constexpr(( IsNativeV<Register<T, N, SIMD>> && ... && impl::NativeExecIsNative<ARGS>::value ))
        {
            return Register<T, N, SIMD> { f( impl::NativeExecSplitOrNative<true>( args )... ) };
        }
        else
        {
            return Register<T, N, SIMD> {
                NativeExec<Register<T, N / 2, SIMD>>( f, impl::NativeExecSplitOrNative<true>( args )... ),
                NativeExec<Register<T, N / 2, SIMD>>( f, impl::NativeExecSplitOrNative<false>( args )... )
            };
        }
    }
}
