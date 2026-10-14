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
#if defined( _MSC_VER ) && !defined( __clang__ )
        // MSVC writes copies of a 32 lane composite mask with two 16 bit stores and reads them back with one 32 bit load,
        // which can't be store forwarded. Copying the native mask keeps the halves in registers
        FS_FORCEINLINE Register( const Register& v ) { this->native = v.native; }
        FS_FORCEINLINE Register& operator =( const Register& v ) { this->native = v.native; return *this; }
#endif
        
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

#if defined( _MSC_VER ) && !defined( __clang__ )
    // MSVC moves both mask halves to general purpose registers to OR them
    template<FastSIMD::FeatureSet SIMD, std::size_t N, bool B, typename = EnableIfNative<Register<Mask<N, B>, 16, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<N, B>, 32, SIMD>& a )
    {
        return !_mm512_kortestz( a.v0.native, a.v1.native );
    }
#endif
}