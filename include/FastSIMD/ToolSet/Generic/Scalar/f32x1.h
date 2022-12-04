#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

#include <algorithm>


namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::enable_if_t<SIMD & FastSIMD::FeatureFlag::Scalar, float>, 1, SIMD>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = float;
        using ElementType = float;
        using MaskType = m32<ElementCount, false, SIMD>;
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
    FS_FORCEINLINE f32<1, SIMD> Load( TypeWrapper<const float*, 1, SIMD> ptr )
    {
        return *ptr.value;
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE void Store( typename f32<1, SIMD>::ElementType* ptr, const f32<1, SIMD>& a )
    {
        *ptr = a.native.f;
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Abs( const f32<1, SIMD>& a )
    {
        return std::abs( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Round( const f32<1, SIMD>& a )
    {
        return std::round( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Ceil( const f32<1, SIMD>& a )
    {
        return std::ceil( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Floor( const f32<1, SIMD>& a )
    {
        return std::floor( a.native.f );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Min( const f32<1, SIMD>& a, const f32<1, SIMD>& b )
    {
        return std::min( a.native.f, b.native.f );        
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Max( const f32<1, SIMD>& a, const f32<1, SIMD>& b )
    {
        return std::max( a.native.f, b.native.f );        
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>>
    FS_FORCEINLINE f32<1, SIMD> Select( const typename f32<1, SIMD>::MaskTypeArg& mask, const f32<1, SIMD>& ifTrue, const f32<1, SIMD>& ifFalse )
    {
        return mask.native ? ifTrue : ifFalse;
    }

}