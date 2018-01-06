#pragma once
#include <inttypes.h>
#include "TypeList.h"

#define FS_CLASS( CLASS ) CLASS ## _SIMD

#define FS_MULTI_TEMPLATE(...) FastSIMD::MultiTemplate<__VAS__>::type

// Vector builders

// Compile time constant
// returns: Number of float32 that will fit into the vector
// I
// int FS_VectorSize_f32()
#define FS_VectorSize_f32 FastSIMD::VectorSize<FS, float>::Count

// Compile time constant
// returns: Number of int32 that will fit into the vector
// I
// int FS_VectorSize_i32()
#define FS_VectorSize_i32 FastSIMD::VectorSize<FS, int32_t>::Count

// returns: Vector with all values set to 0
// I
// float32v FS_VecZero_f32()
#define FS_VecZero_f32 FS::VecZero_f32

// returns: Vector with all values set to 0
// I
// int32v FS_VecZero_i32()
#define FS_VecZero_i32 FS::VecZero_i32

// Compile time constant when using a const value
// value: Value to fill vector with
// returns: Vector with all elements set to given value
// I
// float32v FS_VecFill_f32( float value )
#define FS_VecFill_f32 FS::VecFill_f32

// Compile time constant when using a const value
// value: Value to fill vector with
// returns: Vector with all elements set to given value
// I
// int32v FS_VecFill_i32( int32_t value )
#define FS_VecFill_i32 FS::VecFill_i32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Vec4_f32( float f0, float f1, float f2, float f3 )
#define FS_Vec4_f32 FS::Vec4_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Vec4_i32( int32_t i0, int32_t i1, int32_t i2, int32_t i3 )
#define FS_Vec4_i32 FS::Vec4_i32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Vec4_f32( float f0, float f1, float f2, float f3, float f4, float f5, float f6, float f7 )
#define FS_Vec8_f32 FS::Vec8_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Vec4_i32( int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7 )
#define FS_Vec8_i32 FS::Vec8_i32

// Load

// Copies {vectorSize} bytes from memory location into a vector
// ptr: Pointer to first byte (unaligned)
// returns: Vector with copied data
// I
// float32v FS_Store_f32( void const* ptr )
#define FS_Load_f32 FS::Load_f32

// Copies {vectorSize} bytes from memory location into a vector
// ptr: Pointer to first byte (unaligned)
// returns: Vector with copied data
// I
// int32v FS_Load_i32( void const* ptr )
#define FS_Load_i32 FS::Load_i32


// Store

// Copies all elements of a vector to given memory location
// ptr: Pointer to memory location that elements will be copied to
// vec: Vector to copy from
// I
// void FS_Store_f32( void* ptr, float32v vec )
#define FS_Store_f32 FS::Store_f32

// Copies all elements of a vector to given memory location
// ptr: Pointer to memory location that elements will be copied to
// vec: Vector to copy from
// I
// void FS_Store_i32( void* ptr, int32v vec )
#define FS_Store_i32 FS::Store_i32


// Cast

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Casti32_f32( int32v a )
#define FS_Casti32_f32 FS::Casti32_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Castf32_i32( float32v a )
#define FS_Castf32_i32 FS::Castf32_i32


// Convert

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Converti32_f32( int32v a )
#define FS_Converti32_f32 FS::Converti32_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Convertf32_i32( float32v a )
#define FS_Convertf32_i32 FS::Convertf32_i32


// Comparisons

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_GreaterThan_f32( float32v a, float32v b )
#define FS_GreaterThan_f32 FS::GreaterThan_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_LessThan_f32( float32v a, float32v b )
#define FS_LessThan_f32 FS::LessThan_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_GreaterEqualThan_f32( float32v a, float32v b )
#define FS_GreaterEqualThan_f32 FS::GreaterEqualThan_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_LessEqualThan_f32( float32v a, float32v b )
#define FS_LessEqualThan_f32 FS::LessEqualThan_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_Equal_i32( int32v a, int32v b )
#define FS_Equal_i32 FS::Equal_i32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_GreaterThan_i32( int32v a, int32v b )
#define FS_GreaterThan_i32 FS::GreaterThan_i32

// TEXT
// val: TEXT
// returns: TEXT
// I
// mask32v FS_LessThan_i32( int32v a, int32v b )
#define FS_LessThan_i32 FS::LessThan_i32


// Select

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Select_f32( mask32v m, float32v a, float32v b )
#define FS_Select_f32 FS::Select_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Select_i32( mask32v m, int32v a, int32v b )
#define FS_Select_i32 FS::Select_i32


// Min, Max

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Min_f32( float32v a, float32v b )
#define FS_Min_f32 FS::Min_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Max_f32( float32v a, float32v b )
#define FS_Max_f32 FS::Max_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Min_i32( int32v a, int32v b )
#define FS_Min_i32 FS::Min_i32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Max_i32( int32v a, int32v b )
#define FS_Max_i32 FS::Max_i32


// Bitwise

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_BitwiseAnd_f32( float32v a, float32v b )
#define FS_BitwiseAnd_f32 FS::BitwiseAnd_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_BitwiseOr_f32( float32v a, float32v b )
#define FS_BitwiseOr_f32 FS::BitwiseOr_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_BitwiseXor_f32( float32v a, float32v b )
#define FS_BitwiseXor_f32 FS::BitwiseXor_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_BitwiseNot_f32( float32v a )
#define FS_BitwiseNot_f32 FS::BitwiseNot_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_BitwiseAndNot_f32( float32v a, float32v b )
#define FS_BitwiseAndNot_f32 FS::BitwiseAndNot_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_BitwiseAndNot_i32( int32v a, int32v b )
#define FS_BitwiseAndNot_i32 FS::BitwiseAndNot_i32


// Abs

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Abs_f32( float32v a )
#define FS_Abs_f32 FS::Abs_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// int32v FS_Abs_i32( int32v a )
#define FS_Abs_i32 FS::Abs_i32

// Float math

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Abs_f32( float32v a )
#define FS_Abs_f32 FS::Abs_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Sqrt_f32( float32v a )
#define FS_Sqrt_f32 FS::Sqrt_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_InvSqrt_f32( float32v a )
#define FS_InvSqrt_f32 FS::InvSqrt_f32


// Floor, Ceil, Round

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Floor_f32( float32v a )
#define FS_Floor_f32 FS::Floor_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Ceil_f32( float32v a )
#define FS_Ceil_f32 FS::Ceil_f32

// TEXT
// val: TEXT
// returns: TEXT
// I
// float32v FS_Round_f32( float32v a )
#define FS_Round_f32 FS::Round_f32


namespace FastSIMD
{
    template<typename FS, typename T>
    struct VectorSize
    {
        static_assert((FS::VectorBits / 8) % sizeof( T ) == 0, "Type does not fit into the vector exactly");

        constexpr static int Count()
        {            
            return (FS::VectorBits / 8) / sizeof( T );
        }
    };

    template<typename T>
    struct VectorSize<FastSIMD_Scalar, T>
    {
        constexpr static int Count()
        {
            return 1;
        }
    };

    template<typename FirstT, typename... Types>
    struct MultiTemplate
    {
        typedef typename MultiTemplate<Types...>::type type;
    };

    template<typename... Types>
    struct MultiTemplate<FS_SIMD_CLASS, Types...>
    {
        typedef FS_SIMD_CLASS type;
    };

    template<typename FirstT>
    struct MultiTemplate<FirstT>
    {
        typedef FirstT type;
    };
}
