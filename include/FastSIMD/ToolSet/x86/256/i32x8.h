#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 8, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>>
    {
        static constexpr size_t ElementCount = 8;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = __m256i;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( std::int32_t v ) : native( _mm256_set1_epi32( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = _mm256_add_epi32( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = _mm256_sub_epi32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = _mm256_mullo_epi32( native, rhs.native );            
            return *this;
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = _mm256_and_si256( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = _mm256_or_si256( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = _mm256_xor_si256( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = _mm256_srai_epi32( native, rhs );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = _mm256_slli_epi32( native, rhs );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            const __m256i neg1 = _mm256_set1_epi32( -1 );
            return _mm256_xor_si256( native, neg1 );        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return _mm256_sub_epi32( _mm256_setzero_si256(), native );
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return _mm256_cmpeq_epi32( native, rhs.native );
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
            return _mm256_cmpgt_epi32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return _mm256_cmpgt_epi32( rhs.native, native );
        }

        NativeType native;
    };

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Load( TypeWrapper<const int*, 8, SIMD> ptr )
    {
        return _mm256_loadu_si256( (const __m256i*)ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE void Store( typename i32<8, SIMD>::ElementType* ptr, const i32<8, SIMD>& a )
    {
        _mm256_storeu_si256( (__m256i*)ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE int32_t Extract0( const i32<8, SIMD>& a )
    {
        return _mm256_cvtsi256_si32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Abs( const i32<8, SIMD>& a )
    {
        return _mm256_abs_epi32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Min( const i32<8, SIMD>& a, const i32<8, SIMD>& b )
    {
        return _mm256_min_epi32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Max( const i32<8, SIMD>& a, const i32<8, SIMD>& b )
    {
        return _mm256_max_epi32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Select( const typename i32<8, SIMD>::MaskTypeArg& mask, const i32<8, SIMD>& ifTrue, const i32<8, SIMD>& ifFalse )
    {
        return _mm256_blendv_epi8( ifFalse.native, ifTrue.native, _mm256_castps_si256( mask.native ) );
    }

    template<typename U, FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> SelectHighBit( const Register<U, 8, SIMD>& mask, const i32<8, SIMD>& ifTrue, const i32<8, SIMD>& ifFalse )
    {
        return _mm256_castps_si256( _mm256_blendv_ps( _mm256_castsi256_ps( ifFalse.native ), _mm256_castsi256_ps( ifTrue.native ), FS::Cast<float>( mask ).native ) );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> BitwiseAndNot( const i32<8, SIMD>& a, const i32<8, SIMD>& b )
    {
        return _mm256_andnot_si256( b.native, a.native );        
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> BitShiftRightZeroExtend( const i32<8, SIMD>& a, int b )
    {
        return _mm256_srli_epi32( a.native, b );        
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> Masked( const typename i32<8, SIMD>::MaskTypeArg& mask, const i32<8, SIMD>& a )
    {
        return _mm256_and_si256( _mm256_castps_si256( mask.native ), a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> InvMasked( const typename i32<8, SIMD>::MaskTypeArg& mask, const i32<8, SIMD>& a )
    {
        return _mm256_andnot_si256( _mm256_castps_si256( mask.native ), a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> MaskedIncrement( const typename i32<8, SIMD>::MaskTypeArg& mask, const i32<8, SIMD>& a )
    {
        return _mm256_sub_epi32( a.native, _mm256_castps_si256( mask.native ) );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<8, SIMD>>>
    FS_FORCEINLINE i32<8, SIMD> MaskedDecrement( const typename i32<8, SIMD>::MaskTypeArg& mask, const i32<8, SIMD>& a )
    {
        return _mm256_add_epi32( a.native, _mm256_castps_si256( mask.native ) );    
    }
}
