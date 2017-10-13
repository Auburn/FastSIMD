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


