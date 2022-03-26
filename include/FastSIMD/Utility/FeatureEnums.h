#pragma once
#include "Export.h"

#include <cstdint>

namespace FastSIMD
{
    enum class FeatureFlag
    {
        FMA,

        Scalar,

        x86,
        SSE,
        SSE2,
        SSE3,
        SSSE3,
        SSE41,
        SSE42,
        AVX,
        AVX2,
        AVX512_F,
        AVX512_VL,
        AVX512_DQ,
        AVX512_BW,

        ARM,
        NEON
    };

    constexpr std::uint32_t operator |( FeatureFlag a, FeatureFlag b )
    {
        return 1U << static_cast<std::uint32_t>(a) | 1U << static_cast<std::uint32_t>(b);
    }

    constexpr std::uint32_t operator |( std::uint32_t a, FeatureFlag b )
    {
        return a | 1U << static_cast<std::uint32_t>(b);
    }

    enum class FeatureSet : std::uint32_t
    {
        Null,
        Scalar              =            Null | FeatureFlag::Scalar,
        
        SSE                 =          Scalar | FeatureFlag::x86 | FeatureFlag::SSE,
        SSE2                =             SSE | FeatureFlag::SSE2,
        SSE3                =            SSE2 | FeatureFlag::SSE3,
        SSSE3               =            SSE3 | FeatureFlag::SSSE3,
        SSE41               =           SSSE3 | FeatureFlag::SSE41,
        SSE42               =           SSE41 | FeatureFlag::SSE42,
        AVX                 =           SSE42 | FeatureFlag::AVX,
        AVX_FMA             =             AVX | FeatureFlag::FMA,
        AVX2                =             AVX | FeatureFlag::AVX2,
        AVX2_FMA            =            AVX2 | FeatureFlag::FMA,
        AVX512_Baseline     =            AVX2 | FeatureFlag::AVX512_F | FeatureFlag::AVX512_VL | FeatureFlag::AVX512_DQ | FeatureFlag::AVX512_BW,
        AVX512_Baseline_FMA = AVX512_Baseline | FeatureFlag::FMA,
        
        NEON                =          Scalar | FeatureFlag::ARM | FeatureFlag::NEON,
        NEON_FMA            =            NEON | FeatureFlag::FMA,

        Max = ~0U
    };
    
    constexpr bool operator &( FeatureSet a, FeatureFlag b )
    {
        return static_cast<std::uint32_t>(a) & 1U << static_cast<std::uint32_t>(b);
    }
    
    constexpr bool operator &( FeatureSet a, std::uint32_t b )
    {
        return static_cast<std::uint32_t>(a) & b;
    }    

    FASTSIMD_API FeatureSet CPUMaxFeatureSet();
}