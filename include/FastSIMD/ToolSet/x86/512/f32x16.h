#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 16, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX512_F>>
    {
        static constexpr size_t ElementCount = 16;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __m512;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( _mm512_set1_ps( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm512_add_ps( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm512_sub_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = _mm512_mul_ps( native, rhs.native );
            return *this;           
        }
        
        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            native = _mm512_div_ps( native, rhs.native );
            return *this;           
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm512_and_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm512_or_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm512_xor_ps( native, rhs.native );
            return *this;
        }        

        FS_FORCEINLINE Register operator~() const
        {
            const __m512i neg1 = _mm512_set1_epi32( -1 );
            return _mm512_xor_ps( native, _mm512_castsi512_ps( neg1 ) );        
        }

        FS_FORCEINLINE Register operator-() const
        {
            const __m512i minInt = _mm512_set1_epi32( 0x80000000 );
            return _mm512_xor_ps( native, _mm512_castsi512_ps( minInt ) );
        }
        
        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_EQ_OQ );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_NEQ_OQ );
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_GE_OQ );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_LE_OQ );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_GT_OQ );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm512_cmp_ps_mask( native, rhs.native, _CMP_LT_OQ );
        }

        NativeType native;
    };
    
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Load( TypeWrapper<const float*, 16, SIMD> ptr )
    {
        return _mm512_loadu_ps( ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<16, SIMD>::ElementType* ptr, const f32<16, SIMD>& a )
    {
        _mm512_storeu_ps( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE float Extract0( const f32<16, SIMD>& a )
    {
        return _mm512_cvtss_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Abs( const f32<16, SIMD>& a )
    {
        return _mm512_abs_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Round( const f32<16, SIMD>& a )
    {
        return _mm512_roundscale_ps( a.native, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Floor( const f32<16, SIMD>& a )
    {
        return _mm512_roundscale_ps( a.native, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Ceil( const f32<16, SIMD>& a )
    {
        return _mm512_roundscale_ps( a.native, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Trunc( const f32<16, SIMD>& a )
    {
        return _mm512_roundscale_ps( a.native, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Min( const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_min_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Max( const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_max_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Select( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& ifTrue, const f32<16, SIMD>& ifFalse )
    {
        return _mm512_mask_blend_ps( mask.native, ifFalse.native, ifTrue.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> BitwiseAndNot( const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_andnot_ps( b.native, a.native );        
    }
            
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Masked( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a )
    {
        return _mm512_maskz_mov_ps( mask.native, a.native );    
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> InvMasked( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a )
    {
        return _mm512_maskz_mov_ps( ~mask.native, a.native );    
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> MaskedAdd( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_add_ps( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> MaskedSub( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_sub_ps( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> MaskedMul( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_mul_ps( a.native, mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> InvMaskedAdd( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_add_ps( a.native, ~mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> InvMaskedSub( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_sub_ps( a.native, ~mask.native, a.native, b.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> InvMaskedMul( const typename f32<16, SIMD>::MaskTypeArg& mask, const f32<16, SIMD>& a, const f32<16, SIMD>& b )
    {
        return _mm512_mask_mul_ps( a.native, ~mask.native, a.native, b.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> Reciprocal( const f32<16, SIMD>& a )
    {            
        return _mm512_rcp14_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> InvSqrt( const f32<16, SIMD>& a )
    {            
        return _mm512_rsqrt14_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>>
    FS_FORCEINLINE f32<16, SIMD> Sqrt( const f32<16, SIMD>& a )
    {            
        return _mm512_sqrt_ps( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> FMulAdd( const f32<16, SIMD>& a, const f32<16, SIMD>& b, const f32<16, SIMD>& c )
    {
        return _mm512_fmadd_ps( a.native, b.native, c.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> FMulSub( const f32<16, SIMD>& a, const f32<16, SIMD>& b, const f32<16, SIMD>& c )
    {
        return _mm512_fmsub_ps( a.native, b.native, c.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> FNMulAdd( const f32<16, SIMD>& a, const f32<16, SIMD>& b, const f32<16, SIMD>& c )
    {
        return _mm512_fnmadd_ps( a.native, b.native, c.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<16, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<16, SIMD> FNMulSub( const f32<16, SIMD>& a, const f32<16, SIMD>& b, const f32<16, SIMD>& c )
    {
        return _mm512_fnmsub_ps( a.native, b.native, c.native );
    }
}