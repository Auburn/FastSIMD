//Based on the Qt processor detection code, so should be very accurate
//https: code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/global/qprocessordetection.h

// -- Web Assembly --
#if defined( __EMSCRIPTEN__ )
#define FASTSIMD_ARCH_WASM() 1
#define FASTSIMD_ARCH_NAME() WASM=FASTSIMD_ARCH_WASM()

// -- ARM --
#elif defined( __arm__ ) || defined( __TARGET_ARCH_ARM ) || defined( _M_ARM ) || defined( _M_ARM64 ) || defined( __aarch64__ ) || defined( __ARM64__ )
#if defined( __ARM_ARCH ) && __ARM_ARCH > 1
#define FASTSIMD_ARCH_ARM() __ARM_ARCH
#elif defined( __TARGET_ARCH_ARM ) && __TARGET_ARCH_ARM > 1
#define FASTSIMD_ARCH_ARM() __TARGET_ARCH_ARM
#elif defined( _M_ARM ) && _M_ARM > 1
#define FASTSIMD_ARCH_ARM() _M_ARM
#elif defined( __ARM64_ARCH_8__ ) || defined( __aarch64__ ) || defined( __ARMv8__ ) || defined( __ARMv8_A__ ) || defined( _M_ARM64 )
#define FASTSIMD_ARCH_ARM() 8
#elif defined( __ARM_ARCH_7__ ) || defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7R__ ) || defined( __ARM_ARCH_7M__ ) || defined( __ARM_ARCH_7S__ ) || defined( _ARM_ARCH_7 ) || defined( __CORE_CORTEXA__ )
#define FASTSIMD_ARCH_ARM() 7
#elif defined( __ARM_ARCH_6__ ) || defined( __ARM_ARCH_6J__ ) || defined( __ARM_ARCH_6T2__ ) || defined( __ARM_ARCH_6Z__ ) || defined( __ARM_ARCH_6K__ ) || defined( __ARM_ARCH_6ZK__ ) || defined( __ARM_ARCH_6M__ )
#define FASTSIMD_ARCH_ARM() 6
#elif defined( __ARM_ARCH_5TEJ__ ) || defined( __ARM_ARCH_5TE__ )
#define FASTSIMD_ARCH_ARM() 5
#else
#define FASTSIMD_ARCH_ARM() 1
#endif
#define FASTSIMD_ARCH_NAME() ARM=FASTSIMD_ARCH_ARM()

// -- x86 --
#elif defined( __i386 ) || defined( __i386__ ) || defined( _M_IX86 )
#if defined( _M_IX86 )
#define FASTSIMD_ARCH_X86() ( _M_IX86 / 100 )
#elif defined( __i686__ ) || defined( __athlon__ ) || defined( __SSE__ ) || defined( __pentiumpro__ )
#define FASTSIMD_ARCH_X86() 6
#elif defined( __i586__ ) || defined( __k6__ ) || defined( __pentium__ )
#define FASTSIMD_ARCH_X86() 5
#elif defined( __i486__ ) || defined( __80486__ )
#define FASTSIMD_ARCH_X86() 4
#else
#define FASTSIMD_ARCH_X86() 3
#endif
#define FASTSIMD_ARCH_NAME() X86=FASTSIMD_ARCH_X86()

#elif defined( __x86_64 ) || defined( __x86_64__ ) || defined( __amd64 ) || defined( _M_X64 )
#define FASTSIMD_ARCH_X86() 6
#define FASTSIMD_ARCH_NAME() X86=FASTSIMD_ARCH_X86()

#endif
