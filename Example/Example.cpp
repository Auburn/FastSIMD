
#include "../FastSIMD/FS_Source.inc"
#include "Example.h"

template<typename T>
void FS_CLASS( Example )<T>::DoStuff( int* data )
{
    int32_v a = FS_SetAll_i32( 1 );

    FS_Store_i32( data, a );

    
}

template<>
void FS_CLASS( Example )<FS_MULTI_TEMPLATE( FastSIMD_SSE2 )>::DoStuff( int* data )
{
    int32_v a = _mm_loadu_si128( reinterpret_cast<__m128i const*>( data ) );

    a += _mm_set_epi32( 2, 3, 4, 5 ); 

    a -= _mm_castps_si128( FS_SetAll_f32(1.0f) );

    FS_Store_i32( data, a );
}

FASTSIMD_BUILD_CLASS( Example )
