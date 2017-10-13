
#include "../FastSIMD/FS_Source.inc"
#include "Example.h"

template<typename T>
void FS_CLASS( Example )<T>::DoStuff( int* data )
{
    int32_v a = SetAll_i32( 1 );

    Store_i32( data, a );
}

template<>
void FS_CLASS( Example )<FS_MULTI_TEMPLATE( FastSIMD_SSE2 )>::DoStuff( int* data )
{
    int32_v a = _mm_loadu_si128( reinterpret_cast<__m128i const*>( data ) );

    a += _mm_set_epi32( 2, 3, 4, 5 ); 

    a -= _mm_castps_si128( SetAll_f32(1.0f) );

    Store_i32( data, a );
}

FASTSIMD_BUILD_CLASS( Example )
