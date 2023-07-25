#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<Mask<32, true>, 8, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX>>
    {
        static constexpr size_t ElementCount = 8;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __m256;
        using ElementType = Mask<32, true>;
        using MaskType = Register;
        using MaskTypeArg = Register;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
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
        
        FS_FORCEINLINE Register operator ~() const
        {
            const __m256i neg1 = _mm256_set1_epi32( -1 );
            return _mm256_xor_ps( native, _mm256_castsi256_ps( neg1 ) );        
        }

        NativeType native;
    };

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<m32<8, true, SIMD>>>
    FS_FORCEINLINE m32<8, true, SIMD> BitwiseAndNot( const m32<8, true, SIMD>& a, const m32<8, true, SIMD>& b )
    {
        return _mm256_andnot_ps( b.native, a.native );        
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<m32<8, true, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const m32<8, true, SIMD>& a )
    {          
        return _mm256_movemask_ps( a.native );        
    }
    
    template<FastSIMD::FeatureSet SIMD, bool B, typename = EnableIfNative<m32<8, B, SIMD>>>
    FS_FORCEINLINE BitStorage<8> BitMask( const m32<8, B, SIMD>& a )
    {          
        return static_cast<BitStorage<8>>( _mm256_movemask_ps( a.native ) );        
    }


    template<FastSIMD::FeatureSet SIMD>
    struct Register<Mask<32, false>, 8, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX2>> : Register<Mask<32, true>, 8, SIMD>
    {
        static constexpr size_t ElementCount = 8;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __m256i;
        using ElementType = Mask<32, false>;
        using MaskType = Register;
        using MaskTypeArg = Register<Mask<32, true>, 8, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : Register<Mask<32, true>, 8, SIMD>( _mm256_castsi256_ps( v ) ) { }

        FS_FORCEINLINE NativeType GetNative() const
        {
            return _mm256_castps_si256( this->native );
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            this->native = _mm256_castsi256_ps( _mm256_and_si256( _mm256_castps_si256( this->native ), _mm256_castps_si256( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = _mm256_castsi256_ps( _mm256_or_si256( _mm256_castps_si256( this->native ), _mm256_castps_si256( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = _mm256_castsi256_ps( _mm256_xor_si256( _mm256_castps_si256( this->native ), _mm256_castps_si256( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            const __m256i neg1 = _mm256_set1_epi32( -1 );
            return _mm256_xor_si256( _mm256_castps_si256( this->native ), neg1 );        
        }        
    };

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<m32<8, false, SIMD>>>
    FS_FORCEINLINE m32<8, false, SIMD> BitwiseAndNot( const m32<8, false, SIMD>& a, const m32<8, false, SIMD>& b )
    {
        return _mm256_andnot_si256( _mm256_castps_si256( b.native ), _mm256_castps_si256( a.native ) );        
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<m32<8, false, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const m32<8, false, SIMD>& a )
    {          
        return _mm256_movemask_epi8( _mm256_castps_si256( a.native ) );        
    }
}
