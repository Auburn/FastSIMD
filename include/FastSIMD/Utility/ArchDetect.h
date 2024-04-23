#pragma once
//Based on the Qt processor detection code, so should be very accurate
//https: code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/global/qprocessordetection.h

#define FASTSIMD_ARCH_VALUE_X86() 1
#define FASTSIMD_ARCH_VALUE_ARM() 2
#define FASTSIMD_ARCH_VALUE_WASM() 3

#define FASTSIMD_FEATURE_VALUE_SCALAR() 1

// -- Web Assembly --
#if defined( __EMSCRIPTEN__ ) || defined( EMSCRIPTEN )

#define FASTSIMD_FEATURE_VALUE_WASM() 2

#define FASTSIMD_FEATURE_DETECT() WASM
#define FASTSIMD_ARCH_DETECT() WASM

// -- ARM --
#elif defined( __arm__ ) || defined( __TARGET_ARCH_ARM ) || defined( _M_ARM ) || defined( _M_ARM64 ) || defined( __aarch64__ ) || defined( __ARM64__ )

#define FASTSIMD_FEATURE_VALUE_NEON() 2
#define FASTSIMD_FEATURE_VALUE_AARCH64() 3

#if defined( __ARM64_ARCH_8__ ) || defined( __aarch64__ ) || defined( __ARMv8__ ) || defined( __ARMv8_A__ ) || defined( _M_ARM64 ) || defined( __ARM_NEON__ )
#define FASTSIMD_FEATURE_DETECT() AARCH64
//#elif defined( __ARM_ARCH_7__ ) || defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7R__ ) || defined( __ARM_ARCH_7M__ ) || defined( __ARM_ARCH_7S__ ) || defined( _ARM_ARCH_7 ) || defined( __CORE_CORTEXA__ )
//#define FASTSIMD_ARCH_ARM() 7
//#elif defined( __ARM_ARCH_6__ ) || defined( __ARM_ARCH_6J__ ) || defined( __ARM_ARCH_6T2__ ) || defined( __ARM_ARCH_6Z__ ) || defined( __ARM_ARCH_6K__ ) || defined( __ARM_ARCH_6ZK__ ) || defined( __ARM_ARCH_6M__ )
//#define FASTSIMD_ARCH_ARM() 6
//#elif defined( __ARM_ARCH_5TEJ__ ) || defined( __ARM_ARCH_5TE__ )
//#define FASTSIMD_ARCH_ARM() 5
#else
#define FASTSIMD_FEATURE_DETECT() NEON
#endif

#define FASTSIMD_ARCH_DETECT() ARM

// -- x86 --
#elif defined( __i386 ) || defined( __i386__ ) || defined( _M_IX86 ) || defined( __x86_64 ) || defined( __x86_64__ ) || defined( __amd64 ) || defined( _M_X64 )

#define FASTSIMD_FEATURE_VALUE_SSE() 2
#define FASTSIMD_FEATURE_VALUE_SSE2() 3
#define FASTSIMD_FEATURE_VALUE_SSE3() 4
#define FASTSIMD_FEATURE_VALUE_SSSE3() 5
#define FASTSIMD_FEATURE_VALUE_SSE41() 6
#define FASTSIMD_FEATURE_VALUE_SSE42() 7
#define FASTSIMD_FEATURE_VALUE_AVX() 8
#define FASTSIMD_FEATURE_VALUE_AVX2() 9
#define FASTSIMD_FEATURE_VALUE_AVX512() 10

#if defined( __AVX512F__ ) && defined( __AVX512VL__ ) && defined( __AVX512BW__ ) && defined( __AVX512DQ__ )
#define FASTSIMD_FEATURE_DETECT() AVX512
#elif defined( __AVX2__ )
#define FASTSIMD_FEATURE_DETECT() AVX2
#elif defined( __AVX__ )
#define FASTSIMD_FEATURE_DETECT() AVX
#elif defined( __SSE4_2__ )
#define FASTSIMD_FEATURE_DETECT() SSE42
#elif defined( __SSE4_1__ )
#define FASTSIMD_FEATURE_DETECT() SSE41
#elif defined( __SSSE3__ )
#define FASTSIMD_FEATURE_DETECT() SSSE3
#elif defined( __SSE3__ )
#define FASTSIMD_FEATURE_DETECT() SSE3
#elif defined( __SSE2__ ) || defined( __x86_64 ) || defined( __x86_64__ ) || defined( __amd64 ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP == 2 )
#define FASTSIMD_FEATURE_DETECT() SSE2
#elif defined( __SSE__ ) || defined( __i686__ ) || defined( __athlon__ ) || defined( __pentiumpro__ ) || ( defined( _M_IX86_FP ) && _M_IX86_FP == 1 )
#define FASTSIMD_FEATURE_DETECT() SSE2
#else
#define FASTSIMD_FEATURE_DETECT() SCALAR
#endif

#define FASTSIMD_ARCH_DETECT() X86
#endif


#define FASTSIMD_ARCH_VALUE( arch ) FASTSIMD_ARCH_VALUE_IMPL( arch )
#define FASTSIMD_ARCH_VALUE_IMPL( arch ) FASTSIMD_ARCH_VALUE_##arch()

#define FASTSIMD_CURRENT_ARCH_IS( arch ) FASTSIMD_ARCH_VALUE( FASTSIMD_ARCH_DETECT() ) == FASTSIMD_ARCH_VALUE( arch )


#ifndef FASTSIMD_MAX_FEATURE_SET
#define FASTSIMD_MAX_FEATURE_SET FASTSIMD_FEATURE_DETECT()
#endif
#ifndef FASTSIMD_DEFAULT_FEATURE_SET
#define FASTSIMD_DEFAULT_FEATURE_SET FASTSIMD_MAX_FEATURE_SET
#endif

#define FASTSIMD_FEATURE_VALUE( feature ) FASTSIMD_FEATURE_VALUE_IMPL( feature )
#define FASTSIMD_FEATURE_VALUE_IMPL( feature ) FASTSIMD_FEATURE_VALUE_##feature()

#define FASTSIMD_DEFAULT_FEATURE_VALUE() FASTSIMD_FEATURE_VALUE( FASTSIMD_DEFAULT_FEATURE_SET )
#define FASTSIMD_MAX_FEATURE_VALUE() FASTSIMD_FEATURE_VALUE( FASTSIMD_MAX_FEATURE_SET )
#define FASTSIMD_ARCH_NAME() FASTSIMD_ARCH_DETECT()=FASTSIMD_MAX_FEATURE_SET

#ifndef FASTSIMD_IS_RELAXED
#define FASTSIMD_IS_RELAXED 0
#endif
