#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __m128;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( _mm_set1_ps( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm_add_ps( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm_sub_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = _mm_mul_ps( native, rhs.native );
            return *this;           
        }
        
        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            native = _mm_div_ps( native, rhs.native );
            return *this;           
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm_and_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm_or_ps( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm_xor_ps( native, rhs.native );
            return *this;
        }        

        FS_FORCEINLINE Register operator~() const
        {
            const __m128i neg1 = _mm_set1_epi32( -1 );
            return _mm_xor_ps( native, _mm_castsi128_ps( neg1 ) );        
        }

        FS_FORCEINLINE Register operator-() const
        {
            const __m128i minInt = _mm_set1_epi32( 0x80000000 );
            return _mm_xor_ps( native, _mm_castsi128_ps( minInt ) );
        }
        
        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm_cmpeq_ps( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return _mm_cmpneq_ps( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return _mm_cmpge_ps( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return _mm_cmple_ps( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return _mm_cmpgt_ps( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm_cmplt_ps( native, rhs.native );
        }

        NativeType native;
    };
    
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Load( TypeWrapper<const float*, 4, SIMD> ptr )
    {
        return _mm_loadu_ps( ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<4, SIMD>::ElementType* ptr, const f32<4, SIMD>& a )
    {
        _mm_storeu_ps( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE float Extract0( const f32<4, SIMD>& a )
    {
        return _mm_cvtss_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Abs( const f32<4, SIMD>& a )
    {
        const __m128i intMax = _mm_set1_epi32( 0x7FFFFFFF );
        return _mm_and_ps( a.native, _mm_castsi128_ps( intMax ) );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Round( const f32<4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_round_ps( a.native, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
        }
        else
        {
            __m128i aInt = _mm_cvtps_epi32( a.native );
            __m128 aIntF = _mm_cvtepi32_ps( aInt );

            return _mm_xor_ps( aIntF, _mm_and_ps( _mm_castsi128_ps( _mm_cmpeq_epi32( aInt, _mm_set1_epi32( (-2147483647 - 1) ) ) ), _mm_xor_ps( a.native, aIntF ) ) );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Floor( const f32<4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_round_ps( a.native, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
        }
        else
        {
            f32<4, SIMD> aRound = Round( a );

            return MaskedDecrement( aRound > a, aRound );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Ceil( const f32<4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_round_ps( a.native, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
        }
        else
        {
            f32<4, SIMD> aRound = Round( a );

            return MaskedIncrement( aRound < a, aRound );
        }
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Trunc( const f32<4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_round_ps( a.native, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
        }
        else
        {
            __m128i aInt = _mm_cvttps_epi32( a.native );
            __m128 aIntF = _mm_cvtepi32_ps( aInt );

            return _mm_xor_ps( aIntF, _mm_and_ps( _mm_castsi128_ps( _mm_cmpeq_epi32( aInt, _mm_set1_epi32( (-2147483647 - 1) ) ) ), _mm_xor_ps( a.native, aIntF ) ) );

        }
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Min( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        return _mm_min_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Max( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        return _mm_max_ps( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Select( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& ifTrue, const f32<4, SIMD>& ifFalse )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_blendv_ps( ifFalse.native, ifTrue.native, mask.native );
        }
        else
        {
            return _mm_xor_ps( ifFalse.native, _mm_and_ps( mask.native, _mm_xor_ps( ifTrue.native, ifFalse.native ) ) );
        }
    }

    template<typename U, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE41>>
    FS_FORCEINLINE f32<4, SIMD> SelectHighBit( const Register<U, 4, SIMD>& mask, const f32<4, SIMD>& ifTrue, const f32<4, SIMD>& ifFalse )
    {
        return _mm_blendv_ps( ifFalse.native, ifTrue.native, FS::Cast<float>( mask ).native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> BitwiseAndNot( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        return _mm_andnot_ps( b.native, a.native );        
    }
            
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Masked( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return _mm_and_ps( mask.native, a.native );    
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> InvMasked( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return _mm_andnot_ps( mask.native, a.native );    
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedIncrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return _mm_sub_ps( a.native, _mm_cvtepi32_ps( _mm_castps_si128( mask.native ) ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedDecrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return _mm_add_ps( a.native, _mm_cvtepi32_ps( _mm_castps_si128( mask.native ) ) );
    }

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<4, SIMD> Reciprocal( const f32<4, SIMD>& a )
    {            
        return _mm_rcp_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>()>
    FS_FORCEINLINE f32<4, SIMD> InvSqrt( const f32<4, SIMD>& a )
    {            
        return _mm_rsqrt_ps( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Sqrt( const f32<4, SIMD>& a )
    {            
        return _mm_sqrt_ps( a.native );
    }
}
