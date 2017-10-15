#include "Example.h"

template<typename T>// Generic function, used if no specialised function found
void FS_CLASS( Example )<T>::DoStuff( int* data )
{
    int32v a = FS_SetAll_i32( 1 );

    FS_Store_i32( data, a );
}


template<> // Different function for level SSE2 or AVX2
void FS_CLASS( Example )<FS_MULTI_TEMPLATE( FastSIMD_AVX2, FastSIMD_SSE2 )>::DoStuff( int* data )
{
    int32v a = _mm_loadu_si128( reinterpret_cast<__m128i const*>(data) );

    a += _mm_set_epi32( 2, 3, 4, 5 );

    a -= _mm_castps_si128( FS_SetAll_f32( 1.0f ) );

    FS_Store_i32( data, a );
}


template<typename T>
void FS_CLASS( Example )<T>::DoArray( int* data0, int* data1, int size )
{
    for ( int i = 0; i < size; i += FS_VectorSize_i32() )
    {
        int32v a = FS_Load_i32( &data0[i] );
        int32v b = FS_Load_i32( &data1[i] );

        a += b;

        a <<= 1;

        a -= FS_SetZero_i32();

        a = (~a);

        FS_Store_i32( &data0[i], a );
    }
}
