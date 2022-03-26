#pragma once

#include <FastSIMD/ToolSet/Register.h>

#include <nmmintrin.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE, float>, 4, SIMD>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto SimdFlags = SIMD;

        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( __m128 v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( _mm_set1_ps( v ) ) { }

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
        
        /*FS_FORCEINLINE Register& operator >>=( const Register& rhs )
        {
            native = _mm_srai_epi32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( const Register& rhs )
        {
            native = _mm_slli_epi32( native, rhs.native );
            return *this;
        }*/

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

        __m128 native;
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
    FS_FORCEINLINE f32<4, SIMD> Abs( const f32<4, SIMD>& a )
    {
         const __m128i intMax = _mm_set1_epi32( 0x7FFFFFFF );
        return _mm_and_ps( a.native, _mm_castsi128_ps( intMax ) );
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

}