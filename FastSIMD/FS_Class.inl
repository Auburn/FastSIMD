#undef FASTSIMD_CLASS_DEFINITION
#undef FASTSIMD_CLASS_TYPEDEFS
#undef FS_FUNC_EXTERNAL
#undef FS_INTERNAL

#if !defined( FASTSIMD_INCLUDE_CHECK ) || !defined( FS_SIMD_CLASS )

#define FASTSIMD_CLASS_DEFINITION( CLASS ) \
class CLASS

#define FASTSIMD_CLASS_TYPEDEFS()

#define FS_FUNC_EXTERNAL( FUNC ) \
virtual FUNC = 0

#define FS_INTERNAL( FUNC )

#ifdef FS_SIMD_CLASS
#define FASTSIMD_INCLUDE_CHECK
#endif

#else

#include "Internal/FunctionList.h"

#pragma warning( disable : 868 )

#undef FASTSIMD_INCLUDE_CHECK

#define FASTSIMD_CLASS_DEFINITION( CLASS ) \
template<typename FS_CLASS_T> class FS_CLASS( CLASS ) : public CLASS

#define FASTSIMD_CLASS_TYPEDEFS() \
typedef typename FS_CLASS_T FS;   \
typedef typename FS::float32v float32v;    \
typedef typename FS::int32v   int32v;      \

#define FS_FUNC_EXTERNAL( FUNC ) \
inline FUNC

#define FS_INTERNAL( FUNC ) \
FUNC

#endif
