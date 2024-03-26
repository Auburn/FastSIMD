#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

#include <algorithm>
#include <cmath>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<float, 1, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::Scalar>>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = float;
        using ElementType = float;
        using MaskType = m32<ElementCount, true, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;


        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : native{ v } { }
        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native.f;
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            native.f = native.f + rhs.native.f;
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            native.f = native.f - rhs.native.f;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            native.f = native.f * rhs.native.f;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            native.f = native.f / rhs.native.f;
            return *this;
        }
        
            
        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native.i = native.i & rhs.native.i;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native.i = native.i | rhs.native.i;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native.i = native.i ^ rhs.native.i;    
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            Register reg;
            reg.native.i = ~native.i;
            return reg;        
        }

        FS_FORCEINLINE Register operator -() const
        {
            return -native.f;
        }

        
        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return native.f == rhs.native.f;
        }
        
        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return native.f != rhs.native.f;
        }
        
        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return native.f >= rhs.native.f;
        }
        
        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return native.f <= rhs.native.f;
        }
        
        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return native.f > rhs.native.f;
        }
        
        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return native.f < rhs.native.f;
        }

        union
        {
            float f;
            std::int32_t i;
        }
        native;
    };    
    
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Sqrt( const f32<1, SIMD>& a )
    {
        return std::sqrt( a.native.f );
    }
}
