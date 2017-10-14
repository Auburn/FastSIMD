#pragma once
#include <inttypes.h>

template<typename FS, typename T>
static int32_t VectorCount()
{
    static_assert( (FS::VectorBits / 8) % sizeof( T ) == 0, "Type does not fit into the vector exactly" );

    return (FS::VectorBits / 8) / sizeof( T );
}

/// Returns count of how many (TYPE) will fit into the vector
///
/// Compile time constant
///
/// @param TYPE 
/// Type to check
///
/// @return int32_t
#define FS_VectorCount(TYPE) VectorCount<FS, TYPE>()

// Set all values in the vector
// Compile time constant when using a const value
// I
// float32v FS_SetAll_f32( float value )
#define FS_SetAll_f32 FS::SetAll_f32

// Set all values in the vector
// Compile time constant when using a const value
// I
// int32v FS_SetAll_i32( int32_t value )
#define FS_SetAll_i32 FS::SetAll_i32

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

