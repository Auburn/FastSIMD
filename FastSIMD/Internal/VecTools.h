#pragma once

#include <inttypes.h>

#include "../FastSIMD.h"

#define FS_VECTORCALL __vectorcall
#define FS_INLINE __forceinline
#define FS_ENABLE_IF( CONDITION, TYPE ) typename std::enable_if<(CONDITION), TYPE >::type 

#define FASTSIMD_INTERNAL_TYPE_SET( CLASS, TYPE )                           \
TYPE vector;									                            \
FS_INLINE CLASS( const TYPE& v ) : vector(v) {};	                        \
FS_INLINE CLASS& operator = ( const TYPE& v ) { vector = v; return *this; } \
FS_INLINE operator TYPE() const { return vector; }

#define FASTSIMD_INTERNAL_OPERATOR( TYPE, TYPE2, OPERATOR )	                \
FS_INLINE static TYPE operator OPERATOR ( TYPE lhs, TYPE2 rhs )             \
{											                                \
    lhs OPERATOR= rhs;								                        \
    return lhs;								                                \
}

#define FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE, TYPE2, OPERATOR )	    \
template<FastSIMD::Level L>                                                 \
FS_INLINE static TYPE operator OPERATOR ( TYPE lhs, TYPE2 rhs )             \
{											                                \
    lhs OPERATOR= rhs;								                        \
    return lhs;								                                \
}

#define FASTSIMD_INTERNAL_OPERATORS_FLOAT( TYPE )   \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, + )  \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, - )  \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, * )  \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, / )       

#define FASTSIMD_INTERNAL_OPERATORS_FLOAT_TEMPLATED( TYPE )         \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, + )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, - )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, * )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, / )      

#define FASTSIMD_INTERNAL_OPERATORS_INT( TYPE, TYPE2 ) \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, + )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, - )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, * )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, & )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, | )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, const TYPE&, ^ )     \
FASTSIMD_INTERNAL_OPERATOR( TYPE, TYPE2, >> )          \
FASTSIMD_INTERNAL_OPERATOR( TYPE, TYPE2, << )

#define FASTSIMD_INTERNAL_OPERATORS_INT_TEMPLATED( TYPE, TYPE2 )    \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, + )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, - )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, * )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, & )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, | )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, const TYPE<L>&, ^ )  \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, TYPE2, >> )          \
FASTSIMD_INTERNAL_OPERATOR_TEMPLATED( TYPE<L>, TYPE2, << )