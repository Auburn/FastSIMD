#include "../include/FastSIMD/Utility/ArchDetect.h"

#define TO_LITERAL_( string ) #string
#define TO_LITERAL( string ) TO_LITERAL_( string )

static_assert( 0, "FASTSIMD_ARCH<" TO_LITERAL( FASTSIMD_ARCH_NAME() ) ">" );

// Needed for MacOS clang, it doesn't evaluate macros in static assert errors
TO_LITERAL( FASTSIMD_ARCH<FASTSIMD_ARCH_NAME()> );
