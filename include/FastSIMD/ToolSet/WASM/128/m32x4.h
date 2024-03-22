#pragma once

#include <FastSIMD/ToolSet/Generic/Register.h>

namespace FS
{
    template<FastSIMD::FeatureSet SIMD, bool OPTIMISE_FLOAT>
    struct Register<Mask<32, OPTIMISE_FLOAT>, 4, SIMD, std::enable_if_t<SIMD & FastSIMD::FeatureFlag::WASM>>
    {
        static constexpr size_t ElementCount = 4;
        static constexpr auto FeatureFlags = SIMD;

        using NativeType = v128_t;
        using ElementType = Mask<32, OPTIMISE_FLOAT>;
        using MaskType = Register;
        using MaskTypeArg = Register;

        FS_FORCEINLINE Register() = default;

        template<typename T = NativeType>
        FS_FORCEINLINE Register( std::enable_if_t<OPTIMISE_FLOAT, T> v ) : native( v ) { }

        template<typename T = Register<Mask<32, false>, 4, SIMD>>
        FS_FORCEINLINE Register( const std::enable_if_t<OPTIMISE_FLOAT, T>& v ) : native( v.native ) { }

        FS_FORCEINLINE NativeType GetNative() const
        {
            return native;
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            native = wasm_v128_and( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            native = wasm_v128_or( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            native = wasm_v128_xor( native, rhs.native );
            return *this;
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return wasm_v128_not( native );
        }

        NativeType native;
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
