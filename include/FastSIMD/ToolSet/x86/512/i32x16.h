#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 16, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX512_F>>
    {
        static constexpr size_t ElementCount = 16;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = __m512i;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( std::int32_t v ) : native( _mm512_set1_epi32( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm512_add_epi32( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm512_sub_epi32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = _mm512_mullo_epi32( native, rhs.native );            
            return *this;
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm512_and_si512( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm512_or_si512( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm512_xor_si512( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = _mm512_srai_epi32( native, rhs );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = _mm512_slli_epi32( native, rhs );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            const __m512i neg1 = _mm512_set1_epi32( -1 );
            return _mm512_xor_si512( native, neg1 );        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return _mm512_sub_epi32( _mm512_setzero_si512(), native );
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_EQ );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_NE );
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_NLT );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_LE );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_NLE );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm512_cmp_epi32_mask( native, rhs.native, _MM_CMPINT_LT );
        }

        NativeType native;
    };

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Load( TypeWrapper<const int*, 16, SIMD> ptr )
    {
        return _mm512_loadu_si512( (const __m512i*)ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE void Store( typename i32<16, SIMD>::ElementType* ptr, const i32<16, SIMD>& a )
    {
        _mm512_storeu_si512( (__m512i*)ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE int32_t Extract0( const i32<16, SIMD>& a )
    {
        return _mm512_cvtsi512_si32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Abs( const i32<16, SIMD>& a )
    {
        return _mm512_abs_epi32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Min( const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_min_epi32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Max( const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_max_epi32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Select( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& ifTrue, const i32<16, SIMD>& ifFalse )
    {
        return _mm512_mask_blend_epi32( mask.native, ifFalse.native, ifTrue.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> BitwiseAndNot( const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_andnot_si512( b.native, a.native );        
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> BitShiftRightZeroExtend( const i32<16, SIMD>& a, int b )
    {
        return _mm512_srli_epi32( a.native, b );        
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> Masked( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a )
    {
        return _mm512_maskz_mov_epi32( mask.native, a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> InvMasked( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a )
    {
        return _mm512_maskz_mov_epi32( ~mask.native, a.native );    
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> MaskedAdd( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_add_epi32( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> MaskedSub( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_sub_epi32( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> MaskedMul( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_mullo_epi32( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> InvMaskedAdd( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_add_epi32( a.native, ~mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> InvMaskedSub( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_sub_epi32( a.native, ~mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<16, SIMD>>>
    FS_FORCEINLINE i32<16, SIMD> InvMaskedMul( const typename i32<16, SIMD>::MaskTypeArg& mask, const i32<16, SIMD>& a, const i32<16, SIMD>& b )
    {
        return _mm512_mask_mullo_epi32( a.native, ~mask.native, a.native, b.native );
    }
}