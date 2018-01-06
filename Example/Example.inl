#include "Example.h"
#include "../FastSIMD/FS_IntelliSenseHelper.h"

//template<typename T>// Generic function, used if no specialised function found
//FS_CLASS( Example ) < T, FS_SIMD_CLASS::SIMD_Level >::FS_CLASS( Example )()
//{
//    int test = 1;
//
//    test += test;
//}

template<typename T>// Generic function, used if no specialised function found
void FS_CLASS( Example )<T>::DoStuff( int* data )
{
    int32v a = FS_VecFill_i32( 1 );

    FS_Store_i32( data, a );
}


//template<typename CLASS_T, typename SIMD_T> // Different function for level SSE2 or AVX2
//void FS_CLASS( Example )::DoStuff( int* data )
//{
//    int32v a = _mm_loadu_si128( reinterpret_cast<__m128i const*>(data) );
//
//    a += _mm_set_epi32( 2, 3, 4, 5 );
//
//    a -= _mm_castps_si128( FS_VecZero_f32( ) );
//
//    FS_Store_i32( data, a );
//}
//
//
//template<typename CLASS_T, FastSIMD::Level LEVEL_T>
//void FS_CLASS( Example )::DoArray( int* data0, int* data1, int size )
//{
//    for ( int i = 0; i < size; i += FS_VectorSize_i32() )
//    {
//        int32v a = FS_Load_i32( &data0[i] );
//        int32v b = FS_Load_i32( &data1[i] );
//        
//        a *= b;
//
//        a <<= 1;
//
//        a -= FS_VecZero_i32();
//
//        (~a);
//
//        FS_Store_i32( &data0[i], a );
//    }
//}

template<typename T>
void FS_CLASS( Example )<T>::DoArray( int* data0, int* data1, int size )
{
    for ( int i = 0; i < size; i += FS_VectorSize_i32() )
    {
        int32v a = FS_Load_i32( &data0[i] );
        int32v b = FS_Load_i32( &data1[i] );

        a += b;

        a <<= 1;

        a *= b;

        a -= FS_VecZero_i32();

        (~a);

        FS_Store_i32( &data0[i], a );
    }
}
//
//template<typename T>
//typename std::enable_if<(T::SIMD_Level <= 1)>::type FS_CLASS( Example )<T, FS_SIMD_CLASS::SIMD_Level>::DoArray( int* data0, int* data1, int size )
//{
//    for ( int i = 0; i < size; i += FS_VectorSize_i32() )
//    {
//        int32v a = FS_Load_i32( &data0[i] );
//        int32v b = FS_Load_i32( &data1[i] );
//
//        a += b;
//
//        a <<= 1;
//
//        a -= FS_VecZero_i32();
//
//        (~a);
//
//        FS_Store_i32( &data0[i], a );
//    }
//}
