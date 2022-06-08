#pragma once

#include "Register.h"

#include <algorithm>


namespace FS
{
    template<FastSIMD::FeatureSet SIMD>
    struct Register<std::enable_if_t<SIMD & FastSIMD::FeatureFlag::Scalar, std::int32_t>, 1, SIMD>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;

        using ElementType = std::int32_t;
        using MaskType = m32<ElementCount, false, SIMD>;
        using MaskTypeArg = m32<ElementCount, true, SIMD>;


        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( std::int32_t v ) : native( v ) { }

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
        
        FS_FORCEINLINE Register& operator >>=( const Register& rhs )
        {
            native = native >> rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator <<=( const Register& rhs )
        {
            native = native << rhs.native;
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

        std::int32_t native;
    };
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Abs( const i32<1, SIMD>& a )
    {
        return std::abs( a.native );
    }
    
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Min( const i32<1, SIMD>& a, const i32<1, SIMD>& b )
    {
        return std::min( a.native, b.native );        
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Max( const i32<1, SIMD>& a, const i32<1, SIMD>& b )
    {
        return std::max( a.native, b.native );        
    }
        
    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<i32<1, SIMD>>>
    FS_FORCEINLINE i32<1, SIMD> Select( const typename i32<1, SIMD>::MaskTypeArg& mask, const i32<1, SIMD>& ifTrue, const i32<1, SIMD>& ifFalse )
    {
        return mask.native ? ifTrue : ifFalse;
    }

}