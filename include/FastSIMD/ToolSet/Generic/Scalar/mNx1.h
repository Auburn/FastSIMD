#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, size_t N>
    struct Register<Mask<N, true>, 1, SIMD>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = bool;
        using ElementType = Mask<N, true>;
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
            native = native && rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = native || rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = native ^ rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            return !native;   
        }

        NativeType native;
    };

    template<FastSIMD::FeatureSet SIMD, size_t N, typename = EnableIfNative<Register<Mask<N, true>, 1, SIMD>>>
    FS_FORCEINLINE Register<Mask<N, true>, 1, SIMD> BitwiseAndNot( const Register<Mask<N, true>, 1, SIMD>& a, const Register<Mask<N, true>, 1, SIMD>& b )
    {
        return a.native && !b.native;        
    }
    
    template<FastSIMD::FeatureSet SIMD, size_t N, typename = EnableIfNative<Register<Mask<N, true>, 1, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<N, true>, 1, SIMD>& a )
    {          
        return a.native;
    }

    template<FastSIMD::FeatureSet SIMD, size_t N>
    struct Register<Mask<N, false>, 1, SIMD> : Register<Mask<N, true>, 1, SIMD>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = bool;
        using ElementType = Mask<32, false>;
        using MaskType = Register;
        using MaskTypeArg = Register<Mask<N, true>, 1, SIMD>;

        FS_FORCEINLINE Register() = default;
        FS_FORCEINLINE Register( NativeType v ) : Register<Mask<N, true>, 1, SIMD>( v ) { }
    };
}