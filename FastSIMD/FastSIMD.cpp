#include <inttypes.h>
#include <intrin.h>
#include <stdexcept>

#include "FastSIMD.h"
#include "Internal/TypeList.h"

static FastSIMD::Level simdLevel = FastSIMD::Level_Null;

static_assert( std::is_same<FastSIMD::Level, decltype( FastSIMD::FASTSIMD_FALLBACK_SIMD_LEVEL )>::value, "FASTSIMD_FALLBACK_SIMD_LEVEL not set to a valid SIMD Level, check FastSIMD_Config.h" );
static_assert( FastSIMD::FASTSIMD_FALLBACK_SIMD_LEVEL & FastSIMD::COMPILED_SIMD_LEVELS, "FASTSIMD_FALLBACK_SIMD_LEVEL is not a compiled SIMD level, check FastSIMD_Config.h" );

// Define interface to cpuid instruction.
// input:  eax = functionnumber, ecx = 0
// output: eax = output[0], ebx = output[1], ecx = output[2], edx = output[3]
static void cpuid( int output[4], int functionnumber )
{
#if defined( __GNUC__ ) || defined( __clang__ )              // use inline assembly, Gnu/AT&T syntax

    int a, b, c, d;
    __asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(functionnumber), "c"(0) : );
    output[0] = a;
    output[1] = b;
    output[2] = c;
    output[3] = d;

#elif defined( _MSC_VER ) || defined ( __INTEL_COMPILER )     // Microsoft or Intel compiler, intrin.h included

    __cpuidex( output, functionnumber, 0 ); // intrinsic function for CPUID

#else                                                      // unknown platform. try inline assembly with masm/intel syntax

    __asm
    {
        mov eax, functionnumber
        xor ecx, ecx
        cpuid;
        mov esi, output
            mov[esi], eax
            mov[esi + 4], ebx
            mov[esi + 8], ecx
            mov[esi + 12], edx
    }

#endif
}

// Define interface to xgetbv instruction
static int64_t xgetbv( int ctr )
{
#if (defined( _MSC_FULL_VER ) && _MSC_FULL_VER >= 160040000) || (defined( __INTEL_COMPILER ) && __INTEL_COMPILER >= 1200) // Microsoft or Intel compiler supporting _xgetbv intrinsic

    return _xgetbv( ctr ); // intrinsic function for XGETBV

#elif defined( __GNUC__ )                                    // use inline assembly, Gnu/AT&T syntax

    uint32_t a, d;
    __asm( "xgetbv" : "=a"(a), "=d"(d) : "c"(ctr) : );
    return a | (uint64_t(d) << 32);

#else  // #elif defined (_WIN32)                           // other compiler. try inline assembly with masm/intel/MS syntax

    uint32_t a, d;
    __asm {
        mov ecx, ctr
        _emit 0x0f
        _emit 0x01
        _emit 0xd0; // xgetbv
        mov a, eax
            mov d, edx
    }
    return a | (uint64_t(d) << 32);

#endif
}


FastSIMD::Level FastSIMD::CPUMaxSIMDLevel()
{
    if ( simdLevel > Level_Null )
    {
        return simdLevel;
    }

    simdLevel = Level_Scalar; // default value
    int abcd[4] = {0,0,0,0}; // cpuid results

    cpuid( abcd, 0 ); // call cpuid function 0
    if ( abcd[0] == 0 )
        return simdLevel; // no further cpuid function supported
    cpuid( abcd, 1 ); // call cpuid function 1 for feature flags
    if ( (abcd[3] & (1 << 0)) == 0 )
        return simdLevel; // no floating point
    if ( (abcd[3] & (1 << 23)) == 0 )
        return simdLevel; // no MMX
    if ( (abcd[3] & (1 << 15)) == 0 )
        return simdLevel; // no conditional move
    if ( (abcd[3] & (1 << 24)) == 0 )
        return simdLevel; // no FXSAVE
    if ( (abcd[3] & (1 << 25)) == 0 )
        return simdLevel; // no SSE
    simdLevel = Level_SSE;
    // 1: SSE supported

    if ( (abcd[3] & (1 << 26)) == 0 )
        return simdLevel; // no SSE2
    simdLevel = Level_SSE2;
    // 2: SSE2 supported

    if ( (abcd[2] & (1 << 0)) == 0 )
        return simdLevel; // no SSE3
    simdLevel = Level_SSE3;
    // 3: SSE3 supported

    if ( (abcd[2] & (1 << 9)) == 0 )
        return simdLevel; // no SSSE3
    simdLevel = Level_SSSE3;
    // 4: SSSE3 supported

    if ( (abcd[2] & (1 << 19)) == 0 )
        return simdLevel; // no SSE4.1
    simdLevel = Level_SSE41;
    // 5: SSE4.1 supported

    if ( (abcd[2] & (1 << 23)) == 0 )
        return simdLevel; // no POPCNT
    if ( (abcd[2] & (1 << 20)) == 0 )
        return simdLevel; // no SSE4.2
    simdLevel = Level_SSE42;
    // 6: SSE4.2 supported

    if ( (abcd[2] & (1 << 26)) == 0 )
        return simdLevel; // no XSAVE
    if ( (abcd[2] & (1 << 27)) == 0 )
        return simdLevel; // no OSXSAVE
    if ( (abcd[2] & (1 << 28)) == 0 )
        return simdLevel; // no AVX
    if ( (xgetbv( 0 ) & 6) != 6 )
        return simdLevel; // AVX not enabled in O.S.
    simdLevel = Level_AVX;
    // 7: AVX supported

    cpuid( abcd, 7 ); // call cpuid leaf 7 for feature flags
    if ( (abcd[1] & (1 << 5)) == 0 )
        return simdLevel; // no AVX2
    simdLevel = Level_AVX2;
    // 8: AVX2 supported

    if ( (abcd[1] & (1 << 16)) == 0 )
        return simdLevel; // no AVX512
    cpuid( abcd, 0xD ); // call cpuid leaf 0xD for feature flags
    if ( (abcd[0] & 0x60) != 0x60 )
        return simdLevel; // no AVX512
    simdLevel = Level_AVX512F;
    // 9: AVX512F supported

    cpuid( abcd, 7 ); // call cpuid leaf 7 for feature flags
    if ( (abcd[1] & (1 << 31)) == 0 )
        return simdLevel; // no AVX512VL
    simdLevel = Level_AVX512VL;
    // 10: AVX512VL supported

    if ( (abcd[1] & 0x40020000) != 0x40020000 )
        return simdLevel; // no AVX512BW, AVX512DQ
    simdLevel = Level_AVX512BWDQ;
    // 11: AVX512BW & AVX512DQ supported

    return simdLevel;
}

template<typename CLASS_T, typename SIMD_T>
FS_ENABLE_IF( (CLASS_T::Supported_SIMD_Levels & SIMD_T::SIMD_Level & FastSIMD::COMPILED_SIMD_LEVELS) == 0, CLASS_T* ) ClassFactoryHelper()
{
	return nullptr;
}

template<typename CLASS_T, typename SIMD_T>
FS_ENABLE_IF( (CLASS_T::Supported_SIMD_Levels & SIMD_T::SIMD_Level & FastSIMD::COMPILED_SIMD_LEVELS) != 0, CLASS_T* ) ClassFactoryHelper()
{
	return FastSIMD::ClassFactory<CLASS_T, SIMD_T>::Get();
}


#define FASTSIMD_TRY_LEVEL( CLASS, LEVEL ) \
if ( (LEVEL::SIMD_Level <= maxSIMDLevel || LEVEL::SIMD_Level == FastSIMD::FASTSIMD_FALLBACK_SIMD_LEVEL) && (ptr = ClassFactoryHelper<CLASS, LEVEL>()) ) return ptr;

#define FASTSIMD_BUILD_CLASS( CLASS )                            \
template<>                                                       \
CLASS* FastSIMD::NewSIMDClass<CLASS>( Level maxSIMDLevel )       \
{                                                                \
    CLASS* ptr;                                                  \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_AVX2 )                   \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_SSE42 )                  \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_SSE41 )                  \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_SSSE3 )                  \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_SSE3 )                   \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_SSE2 )                   \
    FASTSIMD_TRY_LEVEL( CLASS, FastSIMD_Scalar )                 \
                                                                 \
    throw std::logic_error( "FASTSIMD_FALLBACK_SIMD_LEVEL not found in FastSIMD::NewSIMDClass" );\
}                                                                


#define FS_SIMD_CLASS void
#define FASTSIMD_INCLUDE_HEADER_ONLY

#include "FastSIMD_BuildList.h"
