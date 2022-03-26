#pragma once

#include <FastSIMD/ToolSet/Register.h>

#ifdef __GNUG__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif


namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE, Mask<32, true>>, 4, SIMD>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto SimdFlags = SIMD;

        using ElementType = Mask<32, true>;
        using MaskType = Register;
        using MaskTypeArg = Register;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( __m128 v ) : native( v ) { }
        
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
        
        FS_FORCEINLINE Register operator ~() const
        {
            const __m128i neg1 = _mm_set1_epi32( -1 );
            return _mm_xor_ps( native, _mm_castsi128_ps( neg1 ) );        
        }

        __m128 native;
    };

    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::enable_if_t<SIMD & FastSIMD::FeatureFlag::SSE2, Mask<32, false>>, 4, SIMD> : Register<Mask<32, true>, 4, SIMD>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto SimdFlags = SIMD;

        using ElementType = Mask<32, false>;
        using MaskType = Register;
        using MaskTypeArg = Register<Mask<32, true>, 4, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( __m128i v ) : Register<Mask<32, true>, 4, SIMD>( _mm_castsi128_ps( v ) ) { }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            this->native =  _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( this->native ), _mm_castps_si128( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = _mm_castsi128_ps( _mm_or_si128( _mm_castps_si128( this->native ), _mm_castps_si128( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = _mm_castsi128_ps( _mm_xor_si128( _mm_castps_si128( this->native ), _mm_castps_si128( rhs.native ) ) );
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            const __m128i neg1 = _mm_set1_epi32( -1 );
            return _mm_castsi128_ps( _mm_xor_si128( _mm_castps_si128( this->native ), neg1 ) );        
        }        
    };


}