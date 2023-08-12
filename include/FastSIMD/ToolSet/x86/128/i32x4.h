#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE2>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = __m128i;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( std::int32_t v ) : native( _mm_set1_epi32( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm_add_epi32( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm_sub_epi32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
            {
                native = _mm_mullo_epi32( native, rhs.native );
            }
            else
            {
                __m128i mul20 = _mm_mul_epu32( native, rhs.native ); /* mul 2,0*/
                __m128i mul31 = _mm_mul_epu32( _mm_srli_si128( native, 4 ), _mm_srli_si128( rhs.native, 4 ) ); /* mul 3,1 */
                native = _mm_unpacklo_epi32( _mm_shuffle_epi32( mul20, _MM_SHUFFLE( 0, 0, 2, 0 ) ), _mm_shuffle_epi32( mul31, _MM_SHUFFLE( 0, 0, 2, 0 ) ) ); /* shuffle results to [63..0] and pack */                
            }
            return *this;
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm_and_si128( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm_or_si128( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm_xor_si128( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = _mm_srai_epi32( native, rhs );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = _mm_slli_epi32( native, rhs );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            const __m128i neg1 = _mm_set1_epi32( -1 );
            return _mm_xor_si128( native, neg1 );        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return _mm_sub_epi32( _mm_setzero_si128(), native );
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm_cmpeq_epi32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return ~(*this == rhs);
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return ~(*this < rhs);
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return ~(*this > rhs);
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return _mm_cmpgt_epi32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm_cmplt_epi32( native, rhs.native );
        }

        NativeType native;
    };

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Load( TypeWrapper<const int*, 4, SIMD> ptr )
    {
        return _mm_loadu_si128( (__m128i*)ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename i32<4, SIMD>::ElementType* ptr, const i32<4, SIMD>& a )
    {
        _mm_storeu_si128( (__m128i*)ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE int32_t Extract0( const i32<4, SIMD>& a )
    {
        return _mm_cvtsi128_si32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Abs( const i32<4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSSE3 )
        {
            return _mm_abs_epi32( a.native );
        }
        else
        {
            __m128i signMask = _mm_srai_epi32( a.native, 31 );
            return _mm_sub_epi32( _mm_xor_si128( a.native, signMask ), signMask );
        }
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Min( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_min_epi32( a.native, b.native );
        }
        else
        {
            return Select( a < b, a, b );
        }
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Max( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_max_epi32( a.native, b.native );
        }
        else
        {
            return Select( a > b, a, b );
        }
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Select( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& ifTrue, const i32<4, SIMD>& ifFalse )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::SSE41 )
        {
            return _mm_blendv_epi8( ifFalse.native, ifTrue.native, _mm_castps_si128( mask.native ) );
        }
        else
        {
            return _mm_xor_si128( ifFalse.native, _mm_and_si128( _mm_castps_si128( mask.native ), _mm_xor_si128( ifTrue.native, ifFalse.native ) ) );
        }
    }
        
    template<typename U, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE41>>
    FS_FORCEINLINE i32<4, SIMD> SelectHighBit( const Register<U, 4, SIMD>& mask, const i32<4, SIMD>& ifTrue, const i32<4, SIMD>& ifFalse )
    {
        return _mm_castps_si128( _mm_blendv_ps( _mm_castsi128_ps( ifFalse.native ), _mm_castsi128_ps( ifTrue.native ), FS::Cast<float>( mask ).native ) );  
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> BitwiseAndNot( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        return _mm_andnot_si128( b.native, a.native );        
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> BitShiftRightZeroExtend( const i32<4, SIMD>& a, int b )
    {
        return _mm_srli_epi32( a.native, b );        
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Masked( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return _mm_and_si128( _mm_castps_si128( mask.native ), a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> InvMasked( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return _mm_andnot_si128( _mm_castps_si128( mask.native ), a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedIncrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return _mm_sub_epi32( a.native, _mm_castps_si128( mask.native ) );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedDecrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return _mm_add_epi32( a.native, _mm_castps_si128( mask.native ) );    
    }
}
