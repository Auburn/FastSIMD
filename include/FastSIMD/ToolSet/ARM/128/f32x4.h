#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::NEON>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = float32x4_t;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( float v ) : native( vdupq_n_f32( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = vaddq_f32( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = vsubq_f32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = vmulq_f32( native, rhs.native );
            return *this;           
        }
        
        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            if constexpr( SIMD & FastSIMD::FeatureFlag::AARCH64 )
            {
                native = vdivq_f32( native, rhs.native );
            }
            else
            {
                float32x4_t reciprocal = vrecpeq_f32( rhs.native );
                // Additional Netwon-Raphson iteration for accuracy
                reciprocal = vmulq_f32( vrecpsq_f32( rhs.native, reciprocal ), reciprocal );
                reciprocal = vmulq_f32( vrecpsq_f32( rhs.native, reciprocal ), reciprocal );

                native = vmulq_f32( native, reciprocal );
            }

            return *this;           
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32( native ), vreinterpretq_u32_f32( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = vreinterpretq_f32_u32( vorrq_u32( vreinterpretq_u32_f32( native ), vreinterpretq_u32_f32( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = vreinterpretq_f32_u32( veorq_u32( vreinterpretq_u32_f32( native ), vreinterpretq_u32_f32( rhs.native ) ) );
            return *this;
        }        

        FS_FORCEINLINE Register operator~() const
        {
            return vreinterpretq_f32_u32( vmvnq_u32( vreinterpretq_u32_f32( native ) ) );        
        }

        FS_FORCEINLINE Register operator-() const
        {
            return vnegq_f32( native );
        }
        
        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return vceqq_f32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return ~( *this == rhs );
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return vcgeq_f32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return vcleq_f32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return vcgtq_f32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return vcltq_f32( native, rhs.native );
        }

        NativeType native;
    };
    
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Load( TypeWrapper<const float*, 4, SIMD> ptr )
    {
        return vld1q_f32( ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<4, SIMD>::ElementType* ptr, const f32<4, SIMD>& a )
    {
        vst1q_f32( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE float Extract0( const f32<4, SIMD>& a )
    {
        return vgetq_lane_f32( a.native, 0 );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Abs( const f32<4, SIMD>& a )
    {
        return vabsq_f32( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Round( const f32<4, SIMD>& a )
    {
        /*if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_round_ps( a.native, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
        }
        else
        {
            __m128i aInt = _mm_cvtps_epi32( a.native );
            __m128 aIntF = _mm_cvtepi32_ps( aInt );

            return _mm_xor_ps( aIntF, _mm_and_ps( _mm_castsi128_ps( _mm_cmpeq_epi32( aInt, _mm_set1_epi32( (-2147483647 - 1) ) ) ), _mm_xor_ps( a.native, aIntF ) ) );
        }*/
        return vrndnq_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Floor( const f32<4, SIMD>& a )
    {
        return vrndmq_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Ceil( const f32<4, SIMD>& a )
    {
        return vrndpq_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Trunc( const f32<4, SIMD>& a )
    {
        return vrndq_f32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Min( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        return vminq_f32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Max( const f32<4, SIMD>& a, const f32<4, SIMD>& b )
    {
        return vmaxq_f32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Select( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& ifTrue, const f32<4, SIMD>& ifFalse )
    {
        return vbslq_f32( mask.native, ifTrue.native, ifFalse.native );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Masked( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32( a.native ), mask.native ) );  
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedIncrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return vsubq_f32( a.native, vcvtq_f32_s32( vreinterpretq_s32_u32( mask.native ) ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> MaskedDecrement( const typename f32<4, SIMD>::MaskTypeArg& mask, const f32<4, SIMD>& a )
    {
        return vaddq_f32( a.native, vcvtq_f32_s32( vreinterpretq_s32_u32( mask.native ) ) );
    }

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<4, SIMD> Reciprocal( const f32<4, SIMD>& a )
    {
        float32x4_t recip = vrecpeq_f32( a.native );
        return vmulq_f32( recip, vrecpsq_f32( recip, a.native ) );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<4, SIMD> InvSqrt( const f32<4, SIMD>& a )
    {
        float32x4_t rsqrt = vrsqrteq_f32( a.native );      
        return vmulq_f32( rsqrt, vrsqrtsq_f32( vmulq_f32( a.native, rsqrt ), rsqrt ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>>
    FS_FORCEINLINE f32<4, SIMD> Sqrt( const f32<4, SIMD>& a )
    {
        return vsqrtq_f32( a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<4, SIMD> FMulAdd( const f32<4, SIMD>& a, const f32<4, SIMD>& b, const f32<4, SIMD>& c )
    {
        return vmlaq_f32( b.native, c.native, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<4, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<4, SIMD> FNMulAdd( const f32<4, SIMD>& a, const f32<4, SIMD>& b, const f32<4, SIMD>& c )
    {
        return vmlaq_f32( b.native, c.native, a.native );
    }
}
