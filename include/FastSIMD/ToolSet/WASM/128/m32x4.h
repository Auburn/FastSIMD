#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    namespace impl
    {
        struct WasmMaskBase32x4
        {
            v128_t native;
        };
    }

    template<FastSIMD::FeatureSet SIMD, bool OPTIMISE_FLOAT>
    struct Register<Mask<32, OPTIMISE_FLOAT>, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::WASM>>
        : std::conditional_t<OPTIMISE_FLOAT, impl::WasmMaskBase32x4, Register<Mask<32, true>, 4, SIMD>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = decltype(WasmMaskBase32x4::native);
        using ElementType = Mask<32, OPTIMISE_FLOAT>;
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
            this->native = wasm_v128_and( this->native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            this->native = wasm_v128_or( this->native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            this->native = wasm_v128_xor( this->native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return wasm_v128_not( this->native );
        }
    };

    template<FastSIMD::FeatureSet SIMD, bool B, typename = EnableIfNative<Register<Mask<32, B>, 4, SIMD>>>
    FS_FORCEINLINE bool AnyMask( const Register<Mask<32, B>, 4, SIMD>& a )
    {
        return wasm_v128_any_true(a.native);
    }

    template<FastSIMD::FeatureSet SIMD, bool B, typename = EnableIfNative<Register<Mask<32, B>, 4, SIMD>>>
    FS_FORCEINLINE BitStorage<4> BitMask( const Register<Mask<32, B>, 4, SIMD>& a )
    {
        return wasm_i32x4_bitmask(a.native);
    }

}
