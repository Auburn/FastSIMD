#include <FastSIMD/Utility/ArchDetect.h>
#include <FastSIMD/Utility/FeatureEnums.h>

#if FASTSIMD_CURRENT_ARCH_IS( X86 )
#ifdef __GNUG__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

// Define interface to cpuid instruction.
// input:  eax = functionnumber, ecx = 0
// output: eax = output[0], ebx = output[1], ecx = output[2], edx = output[3]
static void cpuid( int output[4], int functionnumber )
{
#if defined( __GNUC__ ) || defined( __clang__ ) // use inline assembly, Gnu/AT&T syntax

    int a, b, c, d;
    __asm( "cpuid"
           : "=a"( a ), "=b"( b ), "=c"( c ), "=d"( d )
           : "a"( functionnumber ), "c"( 0 )
           : );
    output[0] = a;
    output[1] = b;
    output[2] = c;
    output[3] = d;

#elif defined( _MSC_VER ) || defined( __INTEL_COMPILER ) // Microsoft or Intel compiler, intrin.h included

    __cpuidex( output, functionnumber, 0 ); // intrinsic function for CPUID

#else // unknown platform. try inline assembly with masm/intel syntax

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
static uint64_t xgetbv( uint32_t ctr )
{
#if( defined( _MSC_FULL_VER ) && _MSC_FULL_VER >= 160040000 ) || ( defined( __INTEL_COMPILER ) && __INTEL_COMPILER >= 1200 ) // Microsoft or Intel compiler supporting _xgetbv intrinsic

    return _xgetbv( ctr ); // intrinsic function for XGETBV

#elif defined( __GNUC__ ) // use inline assembly, Gnu/AT&T syntax

    uint32_t a, d;
    __asm( "xgetbv"
           : "=a"( a ), "=d"( d )
           : "c"( ctr )
           : );
    return a | ( uint64_t( d ) << 32 );

#else // #elif defined (_WIN32)                           // other compiler. try inline assembly with masm/intel/MS syntax

    uint32_t a, d;
    __asm {
        mov ecx, ctr
        _emit 0x0f
        _emit 0x01
        _emit 0xd0; // xgetbv
        mov a, eax
            mov d, edx
    }
    return a | ( uint64_t( d ) << 32 );

#endif
}
#endif

namespace FastSIMD
{
#if FASTSIMD_CURRENT_ARCH_IS( X86 )
    static std::uint32_t DetectCpuSupportedFlags()
    {
        std::uint32_t supportedFlags = FastSIMD::FeatureFlag::x86 | FastSIMD::FeatureFlag::Scalar;

        //#if FASTSIMD_x86
        int abcd[4] = { 0, 0, 0, 0 }; // cpuid results

        //#if !FASTSIMD_64BIT

        cpuid( abcd, 0 ); // call cpuid function 0
        if( abcd[0] == 0 )
            return supportedFlags; // no further cpuid function supported

        cpuid( abcd, 1 ); // call cpuid function 1 for feature flags
        if( ( abcd[3] >> 0 & 1 ) == 0 )
            return supportedFlags; // no floating point
        if( ( abcd[3] >> 23 & 1 ) == 0 )
            return supportedFlags; // no MMX
        if( ( abcd[3] >> 15 & 1 ) == 0 )
            return supportedFlags; // no conditional move
        if( ( abcd[3] >> 24 & 1 ) == 0 )
            return supportedFlags; // no FXSAVE
        if( ( abcd[3] >> 25 & 1 ) == 0 )
            return supportedFlags; // no SSE
        supportedFlags = supportedFlags | FeatureFlag::SSE;
        // SSE supported

        if( ( abcd[3] >> 26 & 1 ) == 0 )
            return supportedFlags; // no SSE2
        //#else
        cpuid( abcd, 1 ); // call cpuid function 1 for feature flags
        //#endif

        supportedFlags = supportedFlags | FeatureFlag::SSE2; // default value for 64bit
        // SSE2 supported

        if( ( abcd[2] >> 0 & 1 ) == 0 )
            return supportedFlags; // no SSE3
        supportedFlags = supportedFlags | FeatureFlag::SSE3;
        // SSE3 supported

        if( ( abcd[2] >> 9 & 1 ) == 0 )
            return supportedFlags; // no SSSE3
        supportedFlags = supportedFlags | FeatureFlag::SSSE3;
        // SSSE3 supported

        if( ( abcd[2] >> 19 & 1 ) == 0 )
            return supportedFlags; // no SSE4.1
        supportedFlags = supportedFlags | FeatureFlag::SSE41;
        // SSE4.1 supported

        if( ( abcd[2] >> 23 & 1 ) == 0 )
            return supportedFlags; // no POPCNT
        if( ( abcd[2] >> 20 & 1 ) == 0 )
            return supportedFlags; // no SSE4.2
        supportedFlags = supportedFlags | FeatureFlag::SSE42;
        // SSE4.2 supported

        if( ( abcd[2] >> 12 & 1 ) == 1 )
            supportedFlags = supportedFlags | FeatureFlag::FMA;
        // FMA3 supported

        if( ( abcd[2] >> 26 & 1 ) == 0 )
            return supportedFlags; // no XSAVE
        if( ( abcd[2] >> 27 & 1 ) == 0 )
            return supportedFlags; // no OSXSAVE
        if( ( abcd[2] >> 28 & 1 ) == 0 )
            return supportedFlags; // no AVX

        uint64_t osbv = xgetbv( 0 );
        if( ( osbv & 6 ) != 6 )
            return supportedFlags; // AVX not enabled in O.S.
        supportedFlags = supportedFlags | FeatureFlag::AVX;
        // AVX supported

        cpuid( abcd, 7 ); // call cpuid leaf 7 for feature flags
        if( ( abcd[1] >> 5 & 1 ) == 0 )
            return supportedFlags; // no AVX2
        supportedFlags = supportedFlags | FeatureFlag::AVX2;
        // AVX2 supported

        if( ( osbv & ( 0xE0 ) ) != 0xE0 )
            return supportedFlags; // AVX512 not enabled in O.S.
        if( ( abcd[1] >> 16 & 1 ) == 0 )
            return supportedFlags; // no AVX512
        cpuid( abcd, 0xD ); // call cpuid leaf 0xD for feature flags
        if( ( abcd[0] & 0x60 ) != 0x60 )
            return supportedFlags; // no AVX512
        supportedFlags = supportedFlags | FeatureFlag::AVX512_F;
        // AVX512 supported

        cpuid( abcd, 7 ); // call cpuid leaf 7 for feature flags

        if( ( abcd[1] >> 31 & 1 ) == 1 )
            supportedFlags = supportedFlags | FeatureFlag::AVX512_VL;
        // AVX512VL supported

        if( ( abcd[1] >> 17 & 1 ) == 1 )
            supportedFlags = supportedFlags | FeatureFlag::AVX512_DQ;
        // AVX512DQ supported

        if( ( abcd[1] >> 30 & 1 ) == 1 )
            supportedFlags = supportedFlags | FeatureFlag::AVX512_BW;
        // AVX512BW supported

        return supportedFlags;
    }

#elif FASTSIMD_CURRENT_ARCH_IS( ARM )
    static std::uint32_t DetectCpuSupportedFlags()
    {
        std::uint32_t supportedFlags =
            FastSIMD::FeatureFlag::ARM |
            FastSIMD::FeatureFlag::Scalar |
            FastSIMD::FeatureFlag::NEON |
            FastSIMD::FeatureFlag::AARCH64 |
            FastSIMD::FeatureFlag::FMA;

        return supportedFlags;
    }

#elif FASTSIMD_CURRENT_ARCH_IS( WASM )
    static std::uint32_t DetectCpuSupportedFlags()
    {
        std::uint32_t supportedFlags =
            FastSIMD::FeatureFlag::WASM |
            FastSIMD::FeatureFlag::Scalar;

        return supportedFlags;
    }
#endif

    static FeatureSet FeatureSetValues[] =
    {
        FeatureSet::SCALAR,

#if FASTSIMD_CURRENT_ARCH_IS( X86 )
        FeatureSet::SSE,
        FeatureSet::SSE2,
        FeatureSet::SSE3,
        FeatureSet::SSSE3,
        FeatureSet::SSE41,
        FeatureSet::SSE42,
        FeatureSet::AVX,
        FeatureSet::AVX_FMA,
        FeatureSet::AVX2,
        FeatureSet::AVX2_FMA,
        FeatureSet::AVX512,
        FeatureSet::AVX512_FMA,

#elif FASTSIMD_CURRENT_ARCH_IS( ARM )
        FeatureSet::NEON,
        FeatureSet::NEON_FMA,
        FeatureSet::AARCH64,
        FeatureSet::AARCH64_FMA,

#elif FASTSIMD_CURRENT_ARCH_IS( WASM )
        FeatureSet::WASM,
#endif
    };

    FASTSIMD_API FastSIMD::FeatureSet DetectCpuMaxFeatureSet()
    {
        static FastSIMD::FeatureSet cache = []
        {
            std::uint32_t supportedFlags = DetectCpuSupportedFlags();

            FeatureSet maxSupported = FeatureSet::Invalid;

            for( FeatureSet featureSet: FeatureSetValues )
            {
                // Check if feature set contains unsupported flags
                if( ( static_cast<std::uint32_t>( featureSet ) ^ supportedFlags ) & ~supportedFlags )
                {
                    break;
                }

                maxSupported = featureSet;
            }

            return maxSupported;
        }();

        return cache;
    }

    FASTSIMD_API const char* GetFeatureSetString( FeatureSet featureSet )
    {
        switch( featureSet )
        {
            case FeatureSet::Invalid: return "Invalid";
            case FeatureSet::SCALAR: return "SCALAR";
            case FeatureSet::SSE: return "SSE";
            case FeatureSet::SSE2: return "SSE2";
            case FeatureSet::SSE3: return "SSE3";
            case FeatureSet::SSSE3: return "SSSE3";
            case FeatureSet::SSE41: return "SSE4.1";
            case FeatureSet::SSE42: return "SSE4.2";
            case FeatureSet::AVX: return "AVX";
            case FeatureSet::AVX_FMA: return "AVX_FMA";
            case FeatureSet::AVX2: return "AVX2";
            case FeatureSet::AVX2_FMA: return "AVX2_FMA";
            case FeatureSet::AVX512: return "AVX512";
            case FeatureSet::AVX512_FMA: return "AVX512_FMA";
            case FeatureSet::NEON: return "NEON";
            case FeatureSet::NEON_FMA: return "NEON_FMA";
            case FeatureSet::AARCH64: return "AARCH64";
            case FeatureSet::AARCH64_FMA: return "AARCH64_FMA";
            case FeatureSet::WASM: return "WASM";
            case FeatureSet::Max: return "Max";
        }

        return "NAN";
    }
} // namespace FastSIMD
