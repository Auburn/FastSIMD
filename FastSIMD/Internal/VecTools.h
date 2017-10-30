#pragma once

#include <inttypes.h>

#include "../FastSIMD.h"

#define FS_VECTORCALL __vectorcall
#define FS_INLINE __forceinline
#define FS_ENABLE_IF( CONDITION, TYPE ) typename std::enable_if<(CONDITION), TYPE>::type 

#define FASTSIMD_INTERNAL_TYPE_SET( CLASS, TYPE )                               \
TYPE vector;									                                \
CLASS( TYPE const& v ) : vector(v) {};	                                        \
CLASS& FS_VECTORCALL operator = ( TYPE const& v ) { vector = v; return *this; } \
FS_VECTORCALL operator TYPE() const { return vector; }

#define FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE2, OPERATOR )	    \
FS_INLINE static TYPE operator OPERATOR ( TYPE lhs, TYPE2 rhs )             \
{											                                \
    lhs OPERATOR= rhs;								                        \
    return lhs;								                                \
}

#define FASTSIMD_INTERNAL_OPERATORS_FLOAT( TYPE )            \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, + )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, - )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, * )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, / )       
                                             
#define FASTSIMD_INTERNAL_OPERATORS_INT( TYPE, TYPE2 )       \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, + )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, - )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, * )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, & )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, | )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE const&, ^ )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE2, >> )       \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATE( TYPE, TYPE2, << )
