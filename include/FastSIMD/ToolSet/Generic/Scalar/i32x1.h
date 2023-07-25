#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

#include <algorithm>
#include <cmath>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::int32_t, 1, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::Scalar>>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = std::int32_t;
        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;


        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native( v ) { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native = native + rhs.native;
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native = native - rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native = native * rhs.native;
            return *this;
        }
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = native & rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = native | rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = native ^ rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            native = native >> rhs;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            native = native << rhs;
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return ~native;        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return -native;
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return native == rhs.native;
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return native != rhs.native;
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return native >= rhs.native;
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return native <= rhs.native;
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return native > rhs.native;
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return native < rhs.native;
        }

        NativeType native;
    };

    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> BitShiftRightZeroExtend( const i32<1, SIMD>& a, int b )
    {
        return static_cast<std::int32_t>( static_cast<std::uint32_t>( a.native ) >> b );
    }
}
