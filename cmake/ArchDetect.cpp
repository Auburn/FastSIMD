#include "../include/FastSIMD/Utility/ArchDetect.h"

#define VALUE(string) #string
#define TO_LITERAL(string) VALUE(string)

static_assert( 0, "FASTSIMD_ARCH<" TO_LITERAL( FASTSIMD_ARCH_NAME() ) ">" );