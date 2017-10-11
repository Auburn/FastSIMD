#pragma once

#include <inttypes.h>

#include "../FastSIMD.h"

#define FS_VECTORCALL __vectorcall
#define FS_INLINE __forceinline

#define FS_VECTOR_TYPE_SET(CLASS, TYPE)                                 \
TYPE vector;									                              \
CLASS(TYPE v) : vector(v) {};	                                              \
CLASS& FS_VECTORCALL operator = (TYPE const& v) { vector = v; return *this; } \
FS_VECTORCALL operator TYPE() const { return vector; }	                      \
                                                                              
#define FS_VECTOR_OPERATOR_TEMPLATE(TYPE, OPERATOR)	                  \
inline TYPE FS_VECTORCALL operator OPERATOR (TYPE lhs, const TYPE& rhs)       \
{											                                  \
    lhs OPERATOR= rhs;								                          \
    return lhs;								                                  \
}												                              \
                                                                              
#define FS_VECTOR_INT_OPERATOR_TEMPLATE(TYPE, OPERATOR)	              \
inline TYPE FS_VECTORCALL operator OPERATOR (TYPE lhs, int32_t rhs)           \
{											                                  \
    lhs OPERATOR= rhs;								                          \
    return lhs;								                                  \
}												                              \
                                                                              
#define FS_VECTOR_OPERATORS_FLOAT(TYPE)                                 \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, +)                                    \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, -)                                    \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, *)                                    \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, /)                                    \
                                                                              
#define FS_VECTOR_OPERATORS_INT(TYPE)                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, + )                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, - )                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, * )                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, & )                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, | )                                   \
FS_VECTOR_OPERATOR_TEMPLATE(TYPE, ^ )                                   \
FS_VECTOR_INT_OPERATOR_TEMPLATE(TYPE, >>)                               \
FS_VECTOR_INT_OPERATOR_TEMPLATE(TYPE, <<)                               \


// Set all values in the vector
// I
// FS_f32 FS_Set1_f32( float value )
#define FS_Set1_f32 FS::Set1_f32

// Set all values in the vector
// I
// FS_i32 FS_Set1_i32( int32_t value )
#define FS_Set1_i32 FS::Set1_i32

// Copies all elements of the vector to given memory location
// I
// void FS_Store_f32( float* ptr, FS_f32 toStore )
#define FS_Store_f32 FS::Store_f32

// Copies all elements of the vector to given memory location
// I
// void FS_Store_i32( void* ptr, FS_i32 toStore )
#define FS_Store_i32 FS::Store_i32

