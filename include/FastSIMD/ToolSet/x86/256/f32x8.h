#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 8, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX>>
    {
        static constexpr size_t ElementCount = 8;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __m256;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( _mm256_set1_ps( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm256_add_ps( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm256_sub_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = _mm256_mul_ps( native, rhs.native );
            return *this;           
        }
        
        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            native = _mm256_div_ps( native, rhs.native );
            return *this;           
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm256_and_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm256_or_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm256_xor_ps( native, rhs.native );
            return *this;
        }        

        FS_FORCEINLINE Register operator~() const
        {
            const __m256i neg1 = _mm256_set1_epi32( -1 );
            return _mm256_xor_ps( native, _mm256_castsi256_ps( neg1 ) );        
        }

        FS_FORCEINLINE Register operator-() const
        {
            const __m256i minInt = _mm256_set1_epi32( 0x80000000 );
            return _mm256_xor_ps( native, _mm256_castsi256_ps( minInt ) );
        }
        
        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_EQ_OQ );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_NEQ_OQ );
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_GE_OQ );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_LE_OQ );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_GT_OQ );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm256_cmp_ps( native, rhs.native, _CMP_LT_OQ );
        }

        NativeType native;
    };
    
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Load( TypeWrapper<const float*, 8, SIMD> ptr )
    {
        return _mm256_loadu_ps( ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<8, SIMD>::ElementType* ptr, const f32<8, SIMD>& a )
    {
        _mm256_storeu_ps( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE float Extract0( const f32<8, SIMD>& a )
    {
        return _mm256_cvtss_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Abs( const f32<8, SIMD>& a )
    {
        const __m256i intMax = _mm256_set1_epi32( 0x7FFFFFFF );
        return _mm256_and_ps( a.native, _mm256_castsi256_ps( intMax ) );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Round( const f32<8, SIMD>& a )
    {
        return _mm256_round_ps( a.native, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Floor( const f32<8, SIMD>& a )
    {
        return _mm256_round_ps( a.native, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Ceil( const f32<8, SIMD>& a )
    {
        return _mm256_round_ps( a.native, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Trunc( const f32<8, SIMD>& a )
    {
        return _mm256_round_ps( a.native, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Min( const f32<8, SIMD>& a, const f32<8, SIMD>& b )
    {
        return _mm256_min_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Max( const f32<8, SIMD>& a, const f32<8, SIMD>& b )
    {
        return _mm256_max_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Select( const typename f32<8, SIMD>::MaskTypeArg& mask, const f32<8, SIMD>& ifTrue, const f32<8, SIMD>& ifFalse )
    {
        return _mm256_blendv_ps( ifFalse.native, ifTrue.native, mask.native );
    }

    template<typename U, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> SelectHighBit( const Register<U, 8, SIMD>& mask, const f32<8, SIMD>& ifTrue, const f32<8, SIMD>& ifFalse )
    {
        return _mm256_blendv_ps( ifFalse.native, ifTrue.native, FS::Cast<float>( mask ).native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> BitwiseAndNot( const f32<8, SIMD>& a, const f32<8, SIMD>& b )
    {
        return _mm256_andnot_ps( b.native, a.native );        
    }
            
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Masked( const typename f32<8, SIMD>::MaskTypeArg& mask, const f32<8, SIMD>& a )
    {
        return _mm256_and_ps( mask.native, a.native );    
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> InvMasked( const typename f32<8, SIMD>::MaskTypeArg& mask, const f32<8, SIMD>& a )
    {
        return _mm256_andnot_ps( mask.native, a.native );    
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> MaskedIncrement( const typename f32<8, SIMD>::MaskTypeArg& mask, const f32<8, SIMD>& a )
    {
        return _mm256_sub_ps( a.native, _mm256_cvtepi32_ps( _mm256_castps_si256( mask.native ) ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> MaskedDecrement( const typename f32<8, SIMD>::MaskTypeArg& mask, const f32<8, SIMD>& a )
    {
        return _mm256_add_ps( a.native, _mm256_cvtepi32_ps( _mm256_castps_si256( mask.native ) ) );
    }

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<8, SIMD> Reciprocal( const f32<8, SIMD>& a )
    {            
        return _mm256_rcp_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<8, SIMD> InvSqrt( const f32<8, SIMD>& a )
    {            
        return _mm256_rsqrt_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>>
    FS_FORCEINLINE f32<8, SIMD> Sqrt( const f32<8, SIMD>& a )
    {            
        return _mm256_sqrt_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>>
    FS_FORCEINLINE f32<8, SIMD> FMulAdd( const f32<8, SIMD>& a, const f32<8, SIMD>& b, const f32<8, SIMD>& c )
    {            
        return _mm256_fmadd_ps( a.native, b.native, c.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>>
    FS_FORCEINLINE f32<8, SIMD> FMulSub( const f32<8, SIMD>& a, const f32<8, SIMD>& b, const f32<8, SIMD>& c )
    {            
        return _mm256_fmsub_ps( a.native, b.native, c.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>>
    FS_FORCEINLINE f32<8, SIMD> FNMulAdd( const f32<8, SIMD>& a, const f32<8, SIMD>& b, const f32<8, SIMD>& c )
    {            
        return _mm256_fnmadd_ps( a.native, b.native, c.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<8, SIMD>>, typename = EnableIfRelaxed<SIMD>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>>
    FS_FORCEINLINE f32<8, SIMD> FNMulSub( const f32<8, SIMD>& a, const f32<8, SIMD>& b, const f32<8, SIMD>& c )
    {            
        return _mm256_fnmsub_ps( a.native, b.native, c.native );
    }
}
