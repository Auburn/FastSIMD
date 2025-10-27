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

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Sin( const f32<1, SIMD>& a )
    {
        return std::sin( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Cos( const f32<1, SIMD>& a )
    {
        return std::cos( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Log( const f32<1, SIMD>& a )
    {
        return std::log( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Exp( const f32<1, SIMD>& a )
    {
        return std::exp( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Pow( const f32<1, SIMD>& a, const f32<1, SIMD>& b )
    {
        return std::pow( a.native.f, b.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> ACos( const f32<1, SIMD>& a )
    {
        return std::acos( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> ASin( const f32<1, SIMD>& a )
    {
        return std::asin( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> ATan( const f32<1, SIMD>& a )
    {
        return std::atan( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> ATan2( const f32<1, SIMD>& y, const f32<1, SIMD>& x )
    {
        return std::atan2( y.native.f, x.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Tan( const f32<1, SIMD>& a )
    {
        return std::tan( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Log2( const f32<1, SIMD>& a )
    {
        return std::log2( a.native.f );
    }

    template<FastSIMD::FeatureSet SIMD, typename = EnableIfNative<f32<1, SIMD>>, typename = EnableIfRelaxed<SIMD>>
    FS_FORCEINLINE f32<1, SIMD> Exp2( const f32<1, SIMD>& a )
    {
        return std::exp2( a.native.f );
    }
}
