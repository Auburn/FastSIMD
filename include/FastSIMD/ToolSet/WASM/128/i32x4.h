#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::WASM>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = v128_t;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( std::int32_t v ) : native( wasm_i32x4_splat( v ) ) { }

        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = wasm_i32x4_add( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = wasm_i32x4_sub( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = wasm_i32x4_mul( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = wasm_v128_and( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = wasm_v128_or( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = wasm_v128_xor( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = wasm_i32x4_shr( native, rhs );
            return *this;
        }

        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = wasm_i32x4_shl( native, rhs );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return wasm_v128_not( native );
        }

        FS_FORCEINLINE Register operator -() const
        {
            return wasm_i32x4_neg( native );
        }


        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return wasm_i32x4_eq( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return ~(*this == rhs);
        }

        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return wasm_i32x4_ge( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return wasm_i32x4_le( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return wasm_i32x4_gt( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return wasm_i32x4_lt( native, rhs.native );
        }

        NativeType native;
    };


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Load( TypeWrapper<const int*, 4, SIMD> ptr )
    {
        return wasm_v128_load( ptr.value );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename i32<4, SIMD>::ElementType* ptr, const i32<4, SIMD>& a )
    {
        wasm_v128_store( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE int32_t Extract0( const i32<4, SIMD>& a )
    {
        return wasm_i32x4_extract_lane( a.native, 0 );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Abs( const i32<4, SIMD>& a )
    {
        return wasm_i32x4_abs( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Min( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        return wasm_i32x4_min( a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Max( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        return wasm_i32x4_max( a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Select( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& ifTrue, const i32<4, SIMD>& ifFalse )
    {
        return wasm_v128_bitselect( ifTrue.native, ifFalse.native, mask.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> BitShiftRightZeroExtend( const i32<4, SIMD>& a, int b )
    {
        return wasm_u32x4_shr( a.native, b );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Masked( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return wasm_v128_and( mask.native, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedIncrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return wasm_i32x4_sub( a.native, mask.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedDecrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return wasm_i32x4_add( a.native, mask.native );
    }
}
