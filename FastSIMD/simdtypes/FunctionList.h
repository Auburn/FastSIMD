#pragma once

// Set all values in the vector
// Compile time constant when using a const value
// I
// float32_v Set1_f32( float value )
#define SetAll_f32 FS::SetAll_f32

// Set all values in the vector
// Compile time constant when using a const value
// I
// int32_v Set1_i32( int32_t value )
#define SetAll_i32 FS::SetAll_i32

// Copies all elements of the vector to given memory location
// I
// void Store_f32( float* ptr, float32_v toStore )
#define Store_f32 FS::Store_f32

// Copies all elements of the vector to given memory location
// I
// void Store_i32( void* ptr, int32_v toStore )
#define Store_i32 FS::Store_i32

