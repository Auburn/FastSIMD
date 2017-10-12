
#include "../FastSIMD/FS_Source.inc"
#include "Example.h"

template<typename T>
void FS_CLASS( Example )<T>::DoStuff( int* data )
{
    FS_i32 a = FS_Set1_i32( 1 );

    FS_Store_i32( data, a );
}

template<>
void FS_CLASS( Example )<FS_MULTI_TEMPLATE( FastSIMD_Scalar )>::DoStuff( int* data )
{
    FS_i32 a = FS_Set1_i32( 2 );

    FS_Store_i32( data, a );
}

FASTSIMD_BUILD_CLASS( Example )
