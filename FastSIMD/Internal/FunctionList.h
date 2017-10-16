#pragma once
#include <inttypes.h>
#include "TypeList.h"

#define FS_CLASS( CLASS ) CLASS ## _SIMD

#define FS_MULTI_TEMPLATE(...) FastSIMD::MultiTemplate<__VA_ARGS__>::type

/// Compile time constant
/// 
/// @return int:
/// Number of floats that will fit into the vector
#define FS_VectorSize_f32 FastSIMD::VectorSize<FS, float_t>::Count

/// Compile time constant
/// @return int:
/// Number of ints that will fit into the vector
#define FS_VectorSize_i32 FastSIMD::VectorSize<FS, int32_t>::Count

// Set all values in the vector to 0
// I
// float32v FS_VecFill_f32()
#define FS_VecZero_f32 FS::VecZero_f32

// Set all values in the vector to 0
// I
// int32v FS_VecFill_i32()
#define FS_VecZero_i32 FS::VecZero_i32

// Set all values in the vector
// Compile time constant when using a const value
// I
// float32v FS_VecFill_f32( float value )
#define FS_VecFill_f32 FS::VecFill_f32

// Set all values in the vector
// Compile time constant when using a const value
// I
// int32v FS_VecFill_i32( int value )
#define FS_VecFill_i32 FS::VecFill_i32

// Loads data from memory location into the vector
// I
// float32v FS_Load_f32( T* ptr )
#define FS_Load_f32 FS::Load_f32

// Loads data from memory location into the vector
// I
// int32v FS_Load_i32( T* ptr )
#define FS_Load_i32 FS::Load_i32

// Copies all elements of the vector to given memory location
// I
// void FS_Store_f32( T* ptr, float32v toStore )
#define FS_Store_f32 FS::Store_f32

// Copies all elements of the vector to given memory location
// I
// void FS_Store_i32( T* ptr, int32v toStore )
#define FS_Store_i32 FS::Store_i32


namespace FastSIMD
{
    template<typename FS, typename T>
    struct VectorSize
    {
        static int Count()
        {
            static_assert((FS::VectorBits / 8) % sizeof( T ) == 0, "Type does not fit into the vector exactly");

            return (FS::VectorBits / 8) / sizeof( T );
        }
    };

    template<typename T>
    struct VectorSize<FastSIMD_Scalar, T>
    {
        static int Count()
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
