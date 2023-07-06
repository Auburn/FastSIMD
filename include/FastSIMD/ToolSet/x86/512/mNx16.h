#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool OPTIMISE_FLOAT>
    struct Register<Mask<N, OPTIMISE_FLOAT>, 16, SIMD>
    {
        static constexpr size_t ElementCount = 16;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = __mmask16;
        using ElementType = Mask<32, OPTIMISE_FLOAT>;
        using MaskType = Register;
        using MaskTypeArg = Register;

        FS_FORCEINLINE Register() = default;

        template<typename T = NativeType>
        FS_FORCEINLINE Register( std::enable_if_t<OPTIMISE_FLOAT, T> v ) : native( v ) { }

        template<typename T = Register<Mask<N, false>, 16, SIMD>>
        FS_FORCEINLINE Register( const std::enable_if_t<OPTIMISE_FLOAT, T>& v ) : native( v.native ) { }

        
        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = ( native & rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = ( native | rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = ( native ^ rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            return ~native;      
        }

        NativeType native;
    };
    
    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 16, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<N, B>, 16, SIMD>& a )
    {          
        return (bool)a.native;        
    }
    
    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 16, SIMD>>>
    FS_FORCEINLINE BitStorage<16> BitMask( const Register<Mask<N, B>, 16, SIMD>& a )
    {          
        return static_cast<BitStorage<16>>( a.native );
    }
}