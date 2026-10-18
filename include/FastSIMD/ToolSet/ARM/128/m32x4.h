#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    namespace impl
    {
        struct ArmMaskBase32x4
        {
            uint32x4_t native;
        };
    }

    template<FastSIMD::FeatureSet SIMD, bool OPTIMISE_FLOAT>
    struct Register<Mask<32, OPTIMISE_FLOAT>, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::ARM>>
        : std::conditional_t<OPTIMISE_FLOAT, impl::ArmMaskBase32x4, Register<Mask<32, true>, 4, SIMD>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = decltype(ArmMaskBase32x4::native);
        using ElementType = Mask<32, OPTIMISE_FLOAT>;
        using MaskType = Register;
        using MaskTypeArg = Register;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) { this->native = v; }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return this->native;
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            this->native = vandq_u32( this->native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = vorrq_u32( this->native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = veorq_u32( this->native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            return vmvnq_u32( this->native );        
        }
    };

    template<FastSIMD::FeatureSet SIMD, bool B, typename = EnableIfNative<Register<Mask<32, B>, 4, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<32, B>, 4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::AARCH64 )
        {
            return vmaxvq_u32( a.native );
        }
        else
        {
            uint32x2_t tmp = vorr_u32( vget_low_u32( a.native ), vget_high_u32( a.native ) );
            return (bool)vget_lane_u32( vpmax_u32( tmp, tmp ), 0 );
        }
    }

    template<FastSIMD::FeatureSet SIMD, bool B, typename = EnableIfNative<Register<Mask<32, B>, 4, SIMD>>>
    FS_FORCEINLINE BitStorage<4> BitMask( const Register<Mask<32, B>, 4, SIMD>& a )
    {
        if constexpr( SIMD & FastSIMD::FeatureFlag::AARCH64 )
        {
            // Weight each lane by its bit and add across. Reads one bit per lane, not the sign bit, so like AnyMask it relies
            // on each lane being all ones or all zeros
            static const uint32_t weights[4] = { 1, 2, 4, 8 };
            return vaddvq_u32( vandq_u32( a.native, vld1q_u32( weights ) ) );
        }
        else
        {
            // Shift out everything but the sign bits with a 32-bit unsigned shift
            // right.
            uint64x2_t high_bits = vreinterpretq_u64_u32( vshrq_n_u32( a.native, 31 ) );
            // Merge the two pairs together with a 64-bit unsigned shift right + add.
            uint8x16_t paired =
                vreinterpretq_u8_u64( vsraq_n_u64( high_bits, high_bits, 31 ) );
            // Extract the result.
            return vgetq_lane_u8( paired, 0 ) | ( vgetq_lane_u8( paired, 8 ) << 2 );
        }
    }

    // 8 lane AARCH64 masks (2 x native): narrow both halves into one register of 16 bit lanes (the top half of each lane),
    // then weight and add across once instead of once per half. Same all ones or all zeros requirement as the 4 lane version
    template<FastSIMD::FeatureSet SIMD, bool B, typename = std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AARCH64>>
    FS_FORCEINLINE BitStorage<8> BitMask( const Register<Mask<32, B>, 8, SIMD>& a )
    {
        static const uint16_t weights[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
        uint16x8_t lanes = vuzp2q_u16( vreinterpretq_u16_u32( a.v0.native ), vreinterpretq_u16_u32( a.v1.native ) );
        return static_cast<BitStorage<8>>( vaddvq_u16( vandq_u16( lanes, vld1q_u16( weights ) ) ) );
    }

}