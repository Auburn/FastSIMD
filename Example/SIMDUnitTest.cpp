#include "SIMDUnitTest.h"

#include <random>
#include <iostream>
#include <cmath>

#include "../FastSIMD/Internal/FunctionList.h"

std::vector<std::function<void( void* )> > SIMDUnitTest::tests;

const std::size_t TestCount = 67108864;
const std::size_t NonVecMask = ~15;

int  * rndInts0;
int  * rndInts1;
float* rndFloats0;
float* rndFloats1;

float GenNormalFloat( std::mt19937& gen )
{
    union
    {
        float f;
        int32_t i;
    } u;

    do
    {
        u.i = gen();

    } while ( !isnormal( u.f ) );

    return u.f;
}

void SIMDUnitTest::RunAll()
{
    rndInts0 = new int[TestCount];
    rndInts1 = new int[TestCount];
    rndFloats0 = new float[TestCount];
    rndFloats1 = new float[TestCount];

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()

    for ( std::size_t i = 0; i < TestCount; i++ )
    {
        rndInts0[i] = gen();
        rndInts1[i] = gen();
        rndFloats0[i] = GenNormalFloat( gen );
        rndFloats1[i] = GenNormalFloat( gen );
    }

    for ( const auto& test : tests )
    {
        test( nullptr );
    }

    delete[] rndInts0;
    delete[] rndInts1;
    delete[] rndFloats0;
    delete[] rndFloats1;
}

#define SIMD_FUNCTION_TEST( NAME, RETURN_TYPE, FUNC ) SIMD_FUNCTION_TEST_BASE( NAME, RETURN_TYPE, FastSIMD::SIMDClassList::FrontCompiledT, FUNC )

#define SIMD_FUNCTION_TEST_BASE( NAME, RETURN_TYPE, LEVEL, FUNC )                                          \
template<typename T, typename FS>                                                                          \
FS_ENABLE_IF( (std::is_same<void, FS>::value), void ) TestFunction_##NAME( void* baseData = nullptr )      \
{                                                                                                          \
    std::cout << "\n";                                                                                     \
    delete[] baseData;                                                                                     \
}                                                                                                          \
                                                                                                           \
template<typename T, typename FS>                                                                          \
FS_ENABLE_IF( !(std::is_same<void, FS>::value), void ) TestFunction_##NAME( void* baseData = nullptr )     \
{                                                                                                          \
    bool isBase = baseData == nullptr;                                                                     \
                                                                                                           \
    if ( isBase )                                                                                          \
    {                                                                                                      \
        std::cout << #NAME " - Base: " << FS::SIMD_Level;                                                  \
        baseData = new T[TestCount];                                                                       \
    }                                                                                                      \
    else { std::cout << " Testing: " << FS::SIMD_Level; }                                                  \
                                                                                                           \
    if ( FS::SIMD_Level > FastSIMD::CPUMaxSIMDLevel() )                                                    \
    {                                                                                                      \
        std::cout << " CPU N//A: " << FS::SIMD_Level;                                                      \
    }                                                                                                      \
    else                                                                                                   \
    {                                                                                                      \
        T result[FastSIMD::VectorSize<FS, T>::Count()];                                                    \
                                                                                                           \
        for ( std::size_t i = 0; i < TestCount; i += FastSIMD::VectorSize<FS, T>::Count() )                \
        {                                                                                                  \
            FUNC;                                                                                          \
                                                                                                           \
            for ( std::size_t ir = 0; ir < FastSIMD::VectorSize<FS, T>::Count(); ir++ )                    \
            {                                                                                              \
                if ( isBase )                                                                              \
                {                                                                                          \
                    ((T*)baseData)[i + ir] = result[ir];                                                   \
                }                                                                                          \
                else if ( result[ir] != ((T*)baseData)[i + ir] &&                                          \
                    (result[ir] == result[ir] ||                                                           \
                    ((T*)baseData)[i + ir] == ((T*)baseData)[i + ir]) )                                    \
                {                                                                                          \
                    std::cout << "\nFailed: expected: " << ((T*)baseData)[i + ir];                         \
                    std::cout << " actual: " << result[ir] << " index: " << i+ir;                          \
                    std::cout << " ints: " << rndInts0[i + ir] << " : " << rndInts1[i + ir];               \
                    std::cout << " floats: " << rndFloats0[i + ir] << " : " << rndFloats1[i + ir] << "\n"; \
                }                                                                                          \
            }                                                                                              \
        }                                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TestFunction_##NAME<T, FastSIMD::SIMDClassList::GetNextCompiledClassT<FS>>( baseData );                \
}                                                                                                          \
SIMDUnitTest test_##NAME( TestFunction_##NAME<RETURN_TYPE, LEVEL> );


SIMD_FUNCTION_TEST( LoadStore_f32, float, FS_Store_f32( &result, FS_Load_f32( &rndFloats0[i] ) ) )

SIMD_FUNCTION_TEST( LoadStore_i32, int32_t, FS_Store_i32( &result, FS_Load_i32( &rndInts0[i] ) ) )


SIMD_FUNCTION_TEST( Fill_i32, int32_t, FS_Store_i32( &result, FS_VecFill_i32( rndInts0[i & NonVecMask] ) ) )

SIMD_FUNCTION_TEST( Fill_f32, float, FS_Store_f32( &result, FS_VecFill_f32( rndFloats0[i & NonVecMask] ) ) )


SIMD_FUNCTION_TEST( Casti32_f32, float, FS_Store_f32( &result, FS_Casti32_f32( FS_Load_i32( &rndInts0[i] ) ) ) )

SIMD_FUNCTION_TEST( Castf32_i32, int32_t, FS_Store_i32( &result, FS_Castf32_i32( FS_Load_f32( &rndFloats0[i] ) ) ) )

SIMD_FUNCTION_TEST( Converti32_f32, float, FS_Store_f32( &result, FS_Converti32_f32( FS_Load_i32( &rndInts0[i] ) ) ) )

SIMD_FUNCTION_TEST( Convertf32_i32, int32_t, FS_Store_i32( &result, FS_Convertf32_i32( FS_Load_f32( &rndFloats0[i] ) ) ) )


SIMD_FUNCTION_TEST( Equal_f32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_Equal_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( GreaterThan_f32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_GreaterThan_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( LessThan_f32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_LessThan_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( GreaterEqualThan_f32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_GreaterEqualThan_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( LessEqualThan_f32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_LessEqualThan_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( Equal_i32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_Equal_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( GreaterThan_i32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_GreaterThan_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ), FS_VecFill_i32( 1 ) ) ) )

SIMD_FUNCTION_TEST( LessThan_i32, int32_t, FS_Store_i32( &result, FS_Mask_i32( FS_LessThan_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ), FS_VecFill_i32( 1 ) ) ) )


SIMD_FUNCTION_TEST( Select_f32, float, FS_Store_f32( &result, FS_Select_f32( FS_GreaterThan_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ), FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( Select_i32, int32_t, FS_Store_i32( &result, FS_Select_i32( FS_GreaterThan_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ), FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ) ) )


SIMD_FUNCTION_TEST( Min_f32, float, FS_Store_f32( &result, FS_Min_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( Max_f32, float, FS_Store_f32( &result, FS_Max_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( Min_i32, float, FS_Store_i32( &result, FS_Min_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ) ) )

SIMD_FUNCTION_TEST( Max_i32, float, FS_Store_i32( &result, FS_Max_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ) ) )


SIMD_FUNCTION_TEST( BitwiseAnd_f32, float, FS_Store_f32( &result, FS_BitwiseAnd_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( BitwiseOr_f32, float, FS_Store_f32( &result, FS_BitwiseOr_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( BitwiseXor_f32, float, FS_Store_f32( &result, FS_BitwiseXor_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( BitwiseNot_f32, float, FS_Store_f32( &result, FS_BitwiseNot_f32( FS_Load_f32( &rndFloats0[i] ) ) ) )

SIMD_FUNCTION_TEST( BitwiseAndNot_f32, float, FS_Store_f32( &result, FS_BitwiseAndNot_f32( FS_Load_f32( &rndFloats0[i] ), FS_Load_f32( &rndFloats1[i] ) ) ) )

SIMD_FUNCTION_TEST( BitwiseAndNot_i32, float, FS_Store_i32( &result, FS_BitwiseAndNot_i32( FS_Load_i32( &rndInts0[i] ), FS_Load_i32( &rndInts1[i] ) ) ) )


SIMD_FUNCTION_TEST( Abs_f32, float, FS_Store_f32( &result, FS_Abs_f32( FS_Load_f32( &rndFloats0[i] ) ) ) )

SIMD_FUNCTION_TEST( Abs_i32, int32_t, FS_Store_i32( &result, FS_Abs_i32( FS_Load_i32( &rndInts0[i] ) ) ) )

SIMD_FUNCTION_TEST( Sqrt_f32, float, FS_Store_f32( &result, FS_Sqrt_f32( FS_Load_f32( &rndFloats0[i] ) ) ) )

//SIMD_FUNCTION_TEST( InvSqrt_f32, float, FS_Store_f32( &result, FS_InvSqrt_f32( FS_Load_f32( &rndFloats0[i] ) ) ) )


const float MAX_ROUNDING = 8192.0f;

SIMD_FUNCTION_TEST( Floor_f32, float, FS_Store_f32( &result, FS_Floor_f32( FS_Min_f32( FS_VecFill_f32( MAX_ROUNDING ), FS_Max_f32( FS_VecFill_f32( -MAX_ROUNDING ), FS_Load_f32( &rndFloats0[i] ) ) ) ) ) )

SIMD_FUNCTION_TEST( Ceil_f32, float, FS_Store_f32( &result, FS_Ceil_f32( FS_Min_f32( FS_VecFill_f32( MAX_ROUNDING ), FS_Max_f32( FS_VecFill_f32( -MAX_ROUNDING ), FS_Load_f32( &rndFloats0[i] ) ) ) ) ) )

//SIMD_FUNCTION_TEST( Round_f32, float, FS_Store_f32( &result, FS_Round_f32( FS_Min_f32( FS_VecFill_f32( MAX_ROUNDING ), FS_Max_f32( FS_VecFill_f32( -MAX_ROUNDING ), FS_Load_f32( &rndFloats0[i] ) ) ) ) ) )

SIMD_FUNCTION_TEST( Mul_f32, float, FS_Store_f32( &result, FS_Load_f32( &rndFloats0[i] ) * FS_Load_f32( &rndFloats1[i] ) ) )

