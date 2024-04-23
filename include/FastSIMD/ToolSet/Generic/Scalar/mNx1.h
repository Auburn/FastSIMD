#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    namespace impl
    {
        struct GenericMaskBase
        {
            bool native;
        };
    }

    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool OPTIMISE_FLOAT>
    struct Register<Mask<N, OPTIMISE_FLOAT>, 1, SIMD>
        : std::conditional_t<OPTIMISE_FLOAT, impl::GenericMaskBase, Register<Mask<N, true>, 1, SIMD>>
    {
        static constexpr size_t ElementCount = 1;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = bool;
        using ElementType = Mask<N, OPTIMISE_FLOAT>;
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
            this->native = this->native && rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = this->native || rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = this->native ^ rhs.native;
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            return !this->native;   
        }
    };

    template<FastSIMD::FeatureSet SIMD, size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 1, SIMD>>>
    FS_FORCEINLINE Register<Mask<N, B>, 1, SIMD> BitwiseAndNot( const Register<Mask<N, B>, 1, SIMD>& a, const Register<Mask<N, B>, 1, SIMD>& b )
    {
        return a.native && !b.native;        
    }
    
    template<FastSIMD::FeatureSet SIMD, size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 1, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<N, B>, 1, SIMD>& a )
    {          
        return a.native;
    }

    template<FastSIMD::FeatureSet SIMD, size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 1, SIMD>>>
    FS_FORCEINLINE BitStorage<1> BitMask( const Register<Mask<N, B>, 1, SIMD>& a )
    {
        return static_cast<BitStorage<1>>( a.native );
    }
}
