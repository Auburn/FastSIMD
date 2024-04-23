#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::NEON>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = int32x4_t;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        FS_FORCEINLINE Register( std::int32_t v ) : native( vdupq_n_s32( v ) ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = vaddq_s32( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = vsubq_s32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = vmulq_s32( native, rhs.native );
            return *this;
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = vandq_s32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = vorrq_s32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = veorq_s32( native, rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = vshlq_s32( native, vdupq_n_s32( -rhs ) );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = vshlq_s32( native, vdupq_n_s32( rhs ) );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return vmvnq_s32( native );        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return vnegq_s32( native );
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return vceqq_s32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return ~(*this == rhs);
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return vcgeq_s32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return vcleq_s32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return vcgtq_s32( native, rhs.native );
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return vcltq_s32( native, rhs.native );
        }

        NativeType native;
    };

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Load( TypeWrapper<const int*, 4, SIMD> ptr )
    {
        return vld1q_s32( ptr.value );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE void Store( typename i32<4, SIMD>::ElementType* ptr, const i32<4, SIMD>& a )
    {
        vst1q_s32( ptr, a.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE int32_t Extract0( const i32<4, SIMD>& a )
    {
        return vgetq_lane_s32( a.native, 0 );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Abs( const i32<4, SIMD>& a )
    {
        return vabsq_s32( a.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Min( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        return vminq_s32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Max( const i32<4, SIMD>& a, const i32<4, SIMD>& b )
    {
        return vmaxq_s32( a.native, b.native );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Select( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& ifTrue, const i32<4, SIMD>& ifFalse )
    {
        return vbslq_s32( mask.native, ifTrue.native, ifFalse.native );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> BitShiftRightZeroExtend( const i32<4, SIMD>& a, int b )
    {
        return vreinterpretq_s32_u32( vshlq_u32( vreinterpretq_u32_s32( a.native ), vdupq_n_s32( -b ) ) );
    }


    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> Masked( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return vandq_s32( vreinterpretq_s32_u32( mask.native ), a.native );    
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedIncrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return vsubq_s32( a.native, vreinterpretq_s32_u32( mask.native ) );
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<4, SIMD>>>
    FS_FORCEINLINE i32<4, SIMD> MaskedDecrement( const typename i32<4, SIMD>::MaskTypeArg& mask, const i32<4, SIMD>& a )
    {
        return vaddq_s32( a.native, vreinterpretq_s32_u32( mask.native ) );    
    }
}
