#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    namespace impl
    {
        struct AVX512MaskBase
        {
            __mmask16 native;
        };
    }

    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool OPTIMISE_FLOAT>
    struct Register<Mask<N, OPTIMISE_FLOAT>, 16, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::AVX512_F>>
        : std::conditional_t<OPTIMISE_FLOAT, impl::AVX512MaskBase, Register<Mask<N, true>, 16, SIMD>>
    {
        static constexpr size_t ElementCount = 16;
        static constexpr auto FeatureFlags = SIMD;
        
        using NativeType = decltype(AVX512MaskBase::native);
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
            this->native = ( this->native & rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = ( this->native | rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = ( this->native ^ rhs.native );
            return *this;
        }
        
        FS_FORCEINLINE Register operator ~() const
        {
            return ~this->native;      
        }
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