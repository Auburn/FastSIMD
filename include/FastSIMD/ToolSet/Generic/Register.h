#pragma once

#include <type_traits>
#include <cstdint>
#include <utility>
#include <tuple>

#include <FastSIMD/Utility/ArchDetect.h>
#include <FastSIMD/Utility/FeatureEnums.h>

#ifdef _MSC_VER
#define FS_FORCEINLINE __forceinline
#define FS_NEVERINLINE __declspec(noinline)
#else
#define FS_FORCEINLINE __attribute__( ( always_inline ) ) inline
#define FS_NEVERINLINE __attribute__( ( noinline ) )
#endif

#if FASTSIMD_CURRENT_ARCH_IS( WASM )
#define FS_VECTORCALL
#elif defined( __clang__ )
#define FS_VECTORCALL __regcall
#elif defined( _MSC_VER )
#define FS_VECTORCALL __vectorcall
#else
#define FS_VECTORCALL
#endif

namespace FS
{
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault(), typename = void>
    struct Register
    {
        static_assert( SIMD != FastSIMD::FeatureSet::Invalid, "Invalid FeatureSet" );
        static_assert( N > 1, "Unknown Vector Type" );
        static_assert( ( N & ( N - 1 ) ) == 0, "Vector size must be power of 2" );

        static constexpr std::size_t ElementCount = N;
        static constexpr auto FeatureFlags = SIMD;

        using DoubledType = Register<T, N / 2, SIMD>;
        using ElementType = T;
        using MaskType = Register<typename DoubledType::MaskType::ElementType, N, SIMD>;
        using MaskTypeArg = Register<typename DoubledType::MaskTypeArg::ElementType, N, SIMD>;

        Register() = default;
        Register( T v ) : v0( v ), v1( v ) { }
        Register( const DoubledType& v, const DoubledType& u ) : v0( v ), v1( u ) { }

        // Conversion for Mask<N, false> -> Mask<N, true>
        template<typename U>
        FS_FORCEINLINE operator Register<U, N, SIMD>() const
        {
            return Register<U, N, SIMD>{ v0, v1 };
        }

        FS_FORCEINLINE Register& operator +=( const Register& rhs )
        {
            v0 += rhs.v0;
            v1 += rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator -=( const Register& rhs )
        {
            v0 -= rhs.v0;
            v1 -= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator *=( const Register& rhs )
        {
            v0 *= rhs.v0;
            v1 *= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator /=( const Register& rhs )
        {
            v0 /= rhs.v0;
            v1 /= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator &=( const Register& rhs )
        {
            v0 &= rhs.v0;
            v1 &= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator |=( const Register& rhs )
        {
            v0 |= rhs.v0;
            v1 |= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator ^=( const Register& rhs )
        {
            v0 ^= rhs.v0;
            v1 ^= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator >>=( const Register& rhs )
        {
            v0 >>= rhs.v0;
            v1 >>= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator <<=( const Register& rhs )
        {
            v0 <<= rhs.v0;
            v1 <<= rhs.v1;
            return *this;
        }

        FS_FORCEINLINE Register& operator >>=( int rhs )
        {
            v0 >>= rhs;
            v1 >>= rhs;
            return *this;
        }

        FS_FORCEINLINE Register& operator <<=( int rhs )
        {
            v0 <<= rhs;
            v1 <<= rhs;
            return *this;
        }

        FS_FORCEINLINE Register operator -() const
        {
            return Register{ -this->v0, -this->v1 };
        }

        FS_FORCEINLINE Register operator ~() const
        {
            return Register{ ~this->v0, ~this->v1 };
        }


        FS_FORCEINLINE MaskType operator ==( const Register& rhs ) const
        {
            return MaskType{ v0 == rhs.v0, v1 == rhs.v1 };
        }

        FS_FORCEINLINE MaskType operator !=( const Register& rhs ) const
        {
            return MaskType{ v0 != rhs.v0, v1 != rhs.v1 };
        }

        FS_FORCEINLINE MaskType operator >=( const Register& rhs ) const
        {
            return MaskType{ v0 >= rhs.v0, v1 >= rhs.v1 };
        }

        FS_FORCEINLINE MaskType operator <=( const Register& rhs ) const
        {
            return MaskType{ v0 <= rhs.v0, v1 <= rhs.v1 };
        }

        FS_FORCEINLINE MaskType operator >( const Register& rhs ) const
        {
            return MaskType{ v0 > rhs.v0, v1 > rhs.v1 };
        }

        FS_FORCEINLINE MaskType operator <( const Register& rhs ) const
        {
            return MaskType{ v0 < rhs.v0, v1 < rhs.v1 };
        }

        DoubledType v0, v1;
    };

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator +( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs += rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator -( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs -= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator *( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs *= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator /( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs /= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator &( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs &= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator |( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs |= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator ^( Register<T, N, SIMD> lhs, const Register<T, N, SIMD>& rhs )
    {
        return lhs ^= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator <<( Register<T, N, SIMD> lhs, int rhs )
    {
        return lhs <<= rhs;
    }

    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator >>( Register<T, N, SIMD> lhs, int rhs )
    {
        return lhs >>= rhs;
    }

    template<typename T, typename U, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator<<( Register<T, N, SIMD> lhs, Register<U, N, SIMD> rhs )
    {
        return lhs <<= rhs;
    }

    template<typename T, typename U, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE static Register<T, N, SIMD> operator>>( Register<T, N, SIMD> lhs, Register<U, N, SIMD> rhs )
    {
        return lhs >>= rhs;
    }

    template<typename T, std::size_t N = 0, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    struct TypeWrapper
    {
        using Type = T;
        using Half = TypeWrapper<T, N / 2, SIMD>;

        FS_FORCEINLINE constexpr explicit TypeWrapper( T v ) : value( v ) { }

        FS_FORCEINLINE constexpr Half AsHalf() const
        {
            return Half( value );
        }

        template<typename U>
        FS_FORCEINLINE constexpr Half AsHalf( U offset ) const
        {
            return Half( value + offset );
        }

        T value;
    };

    template<typename T>
    struct TypeDummy
    {
        using Type = T;
    };

    template<std::size_t N, bool OPTIMISE_FLOAT = true>
    struct Mask
    {
        Mask() = delete;
    };

    template<typename T, typename = void>
    struct IsNative : std::false_type { };

    template<typename T>
    struct IsNative<T, std::void_t<typename T::NativeType>> : std::true_type { };

    template<typename T>
    using EnableIfNative = typename T::NativeType;

    template<typename T>
    using EnableIfNotNative = decltype( T::v0 );

    template<typename T>
    constexpr bool IsNativeV = IsNative<T>::value;

    template<auto SIMD = 0>
    using EnableIfRelaxed = std::enable_if_t<FastSIMD::IsRelaxed<SIMD>()>;

    template<auto SIMD = 0>
    using EnableIfNotRelaxed = std::enable_if_t<!FastSIMD::IsRelaxed<SIMD>()>;


    template<std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    using i32 = Register<std::int32_t, N, SIMD>;

    template<std::size_t N, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    using f32 = Register<float, N, SIMD>;

    template<std::size_t N, bool OPTIMISE_FLOAT = true, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    using m32 = Register<Mask<32, OPTIMISE_FLOAT>, N, SIMD>;

    template<std::size_t N>
    using BitStorage = std::tuple_element_t<( N > 8 ) + ( N > 16 ) + ( N > 32 ),
                                            std::tuple<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>>;

    template<typename T>
    static constexpr std::size_t NativeRegisterCount( FastSIMD::FeatureSet featureSet = FastSIMD::FeatureSetDefault() );

    template<>
    constexpr std::size_t NativeRegisterCount<float>( FastSIMD::FeatureSet featureSet )
    {
        if( featureSet & FastSIMD::FeatureFlag::AVX512_F )
        {
            return 16;
        }
        if( featureSet & FastSIMD::FeatureFlag::AVX )
        {
            return 8;
        }
        if( featureSet & (FastSIMD::FeatureFlag::SSE |
            FastSIMD::FeatureFlag::NEON | FastSIMD::FeatureFlag::WASM) )
        {
            return 4;
        }

        return 1;
    }

    template<>
    constexpr std::size_t NativeRegisterCount<std::int32_t>( FastSIMD::FeatureSet featureSet )
    {
        if( featureSet & FastSIMD::FeatureFlag::AVX512_F )
        {
            return 16;
        }
        if( featureSet & FastSIMD::FeatureFlag::AVX2 )
        {
            return 8;
        }
        if( featureSet & (FastSIMD::FeatureFlag::SSE2 |
            FastSIMD::FeatureFlag::NEON | FastSIMD::FeatureFlag::WASM) )
        {
            return 4;
        }

        return 1;
    }

    template<>
    constexpr std::size_t NativeRegisterCount<Mask<32>>( FastSIMD::FeatureSet featureSet )
    {
        if( featureSet & FastSIMD::FeatureFlag::AVX512_F )
        {
            return 16;
        }
        if( featureSet & FastSIMD::FeatureFlag::AVX2 )
        {
            return 8;
        }
        if( featureSet & (FastSIMD::FeatureFlag::SSE2 |
            FastSIMD::FeatureFlag::NEON | FastSIMD::FeatureFlag::WASM) )
        {
            return 4;
        }

        return 1;
    }

    template<typename T, FastSIMD::FeatureSet SIMD = FastSIMD::FeatureSetDefault()>
    using NativeRegister = Register<T, NativeRegisterCount<T>( SIMD ), SIMD>;

} // namespace FS
