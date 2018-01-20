#pragma once

#include "../FastSIMD.h"

template<FastSIMD::Level L>
struct DummySIMDClass
{
    static const FastSIMD::Level SIMD_Level = L;        
};

#ifdef FASTSIMD_COMPILE_SCALAR
#include "Scalar.h"
#else
typedef DummySIMDClass<FastSIMD::Level_Scalar> FastSIMD_Scalar
#endif

#if FASTSIMD_COMPILE_SSE | FASTSIMD_COMPILE_SSE2 | FASTSIMD_COMPILE_SSE3 | FASTSIMD_COMPILE_SSSE3 | FASTSIMD_COMPILE_SSE41 | FASTSIMD_COMPILE_SSE42
#include "SSE.h"
#endif
#if !(FASTSIMD_COMPILE_SSE)
typedef DummySIMDClass<FastSIMD::Level_SSE> FastSIMD_SSE;
#endif
#if !(FASTSIMD_COMPILE_SSE2)
typedef DummySIMDClass<FastSIMD::Level_SSE2> FastSIMD_SSE2;
#endif
#if !(FASTSIMD_COMPILE_SSE3)
typedef DummySIMDClass<FastSIMD::Level_SSE3> FastSIMD_SSE3;
#endif
#if !(FASTSIMD_COMPILE_SSSE3)
typedef DummySIMDClass<FastSIMD::Level_SSSE3> FastSIMD_SSSE3;
#endif
#if !(FASTSIMD_COMPILE_SSE41)
typedef DummySIMDClass<FastSIMD::Level_SSE41> FastSIMD_SSE41;
#endif
#if !(FASTSIMD_COMPILE_SSE42)
typedef DummySIMDClass<FastSIMD::Level_SSE42> FastSIMD_SSE42;
#endif

#if FASTSIMD_COMPILE_AVX | FASTSIMD_COMPILE_AVX2
#include "AVX.h"
#endif
#if !(FASTSIMD_COMPILE_AVX)
typedef DummySIMDClass<FastSIMD::Level_AVX> FastSIMD_AVX;
#endif
#if !(FASTSIMD_COMPILE_AVX2)
typedef DummySIMDClass<FastSIMD::Level_AVX2> FastSIMD_AVX2;
#endif

namespace FastSIMD
{
    template<typename... T>
    struct SIMDClassContainer
    {
        using FrontT = void;

        using FrontCompiledT = void;

        template<Level L>
        using GetByLevelT = void;

        template<typename CLASS>
        using GetFollowingClassT = void;

        template<typename CLASS>
        using GetNextCompiledClassT = void;
    };

    template<typename HEAD, typename... TAIL>
    struct SIMDClassContainer<HEAD, TAIL...>
    {
        using FrontT = HEAD;

        using FrontCompiledT = typename std::conditional<(HEAD::SIMD_Level & COMPILED_SIMD_LEVELS) != 0, HEAD, typename SIMDClassContainer<TAIL...>::FrontCompiledT>::type;

        template<Level L>
        using GetByLevelT = typename std::conditional< L == HEAD::SIMD_Level, HEAD, typename SIMDClassContainer<TAIL...>::template GetByLevelT<L> >::type;

        template<typename CLASS>
        using GetFollowingClassT = typename std::conditional< std::is_same<CLASS, HEAD>::value, typename SIMDClassContainer<TAIL...>::FrontT, typename SIMDClassContainer<TAIL...>::template GetFollowingClassT<CLASS> >::type;

        template<typename CLASS>
        using GetNextCompiledClassT = typename std::conditional< std::is_same<CLASS, HEAD>::value, typename SIMDClassContainer<TAIL...>::FrontCompiledT, typename SIMDClassContainer<TAIL...>::template GetNextCompiledClassT<CLASS> >::type;
    };

    typedef SIMDClassContainer<
        FastSIMD_Scalar,
        FastSIMD_SSE,
        FastSIMD_SSE2,
        FastSIMD_SSE3,
        FastSIMD_SSSE3,
        FastSIMD_SSE41,
        FastSIMD_SSE42,
        FastSIMD_AVX,
        FastSIMD_AVX2
    >
        SIMDClassList;

    static_assert( SIMDClassList::FrontCompiledT::SIMD_Level == FASTSIMD_FALLBACK_SIMD_LEVEL, "FASTSIMD_FALLBACK_SIMD_LEVEL is not the lowest compiled SIMD level" );
}