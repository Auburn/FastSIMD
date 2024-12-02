#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::WASM>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = __f32x4;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( wasm_f32x4_splat( v ) ) { }

        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = wasm_f32x4_add( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = wasm_f32x4_sub( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = wasm_f32x4_mul( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            native = wasm_f32x4_div( native, rhs.native );
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

        FS_FORCEINLINE Register operator~() const
        {
            return wasm_v128_not( native );
        }

        FS_FORCEINLINE Register operator-() const
        {
            return wasm_f32x4_neg( native );
        }


        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return wasm_f32x4_eq( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return ~( *this == rhs );
        }

        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return wasm_f32x4_ge( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return wasm_f32x4_le( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return wasm_f32x4_gt( native, rhs.native );
        }

        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return wasm_f32x4_lt( native, rhs.native );
        }

        NativeType native;
    };


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Load( TypeWrapper<const float*, 4, SIMD> ptr )
    {
        return wasm_v128_load( ptr.value );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<4, SIMD>::ElementType* ptr, const f32<4, SIMD>& a )
    {
        wasm_v128_store( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE float Extract0( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_extract_lane( a.native, 0 );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Abs( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_abs( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Round( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_nearest( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Floor( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_floor( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Trunc( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_trunc( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Ceil( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_ceil( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Min( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        if constexpr( FastSIMD::IsRelaxed() )
        {
            return wasm_f32x4_relaxed_min( a.native, b.native );        
        }
        else
        {
            return wasm_f32x4_min( a.native, b.native );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Max( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        if constexpr( FastSIMD::IsRelaxed() )
        {
            return wasm_f32x4_relaxed_max( a.native, b.native );
        }
        else
        {
            return wasm_f32x4_max( a.native, b.native );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Select( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& ifTrue, const f32<4, SIMD>& ifFalse )
    {
        return wasm_v128_bitselect( ifTrue.native, ifFalse.native, mask.native );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Masked( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return wasm_v128_and( mask.native, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedIncrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return wasm_f32x4_sub( a.native,
            wasm_f32x4_convert_i32x4( static_cast<v128_t>( mask.native ) ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedDecrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return wasm_f32x4_add( a.native,
            wasm_f32x4_convert_i32x4( static_cast<v128_t>( mask.native ) ) );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Sqrt( const f32<4, SIMD>& a )
    {
        return wasm_f32x4_sqrt( a.native );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<4, SIMD> FMulAdd( const f32<4, SIMD>& a, const f32<4, SIMD>& b, const f32<4, SIMD>& c )
    {
        return wasm_f32x4_relaxed_madd( a.native, b.native, c.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<4, SIMD> FNMulAdd( const f32<4, SIMD>& a, const f32<4, SIMD>& b, const f32<4, SIMD>& c )
    {
        return wasm_f32x4_relaxed_nmadd( a.native, b.native, c.native );
    }
}
