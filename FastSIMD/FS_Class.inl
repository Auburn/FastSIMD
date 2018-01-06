#undef FASTSIMD_CLASS_DEFINITION
#undef FASTSIMD_CLASS_SETUP
#undef FASTSIMD_SET_SUPPORTED_SIMD_LEVELS
#undef FS_FUNC_EXTERNAL
#undef FS_INTERNAL

#if !defined( FASTSIMD_INCLUDE_CHECK ) || !defined( FS_SIMD_CLASS )

#define FASTSIMD_CLASS_DEFINITION( CLASS ) \
class CLASS

#define FASTSIMD_CLASS_SETUP()\
virtual FastSIMD::Level GetSIMDLevel() = 0

#define FASTSIMD_SET_SUPPORTED_SIMD_LEVELS( LEVELS ) \
static const unsigned int Supported_SIMD_Levels = ( LEVELS );\
static_assert( (Supported_SIMD_Levels & FastSIMD::FASTSIMD_FALLBACK_SIMD_LEVEL) != 0, "Fallback SIMD level must be supported" );

#define FS_FUNC_EXTERNAL( FUNC ) \
virtual FUNC = 0

#define FS_INTERNAL( FUNC )

#ifdef FS_SIMD_CLASS
#define FASTSIMD_INCLUDE_CHECK
#endif

#else

#include "Internal/FunctionList.h"
#include "FastSIMD.h"

#undef FASTSIMD_INCLUDE_CHECK

#define FASTSIMD_CLASS_DEFINITION( CLASS ) \
template<typename FS_CLASS_T> class FS_CLASS( CLASS ) : public CLASS
//template<typename FS_CLASS_T> class FS_CLASS( CLASS )<FS_CLASS_T, FS_SIMD_CLASS::SIMD_Level> 

#define FASTSIMD_CLASS_SETUP() \
typedef typename FS_CLASS_T FS;   \
typedef typename FS::float32v float32v;    \
typedef typename FS::int32v   int32v;      \
FastSIMD::Level GetSIMDLevel() override { return FS::SIMD_Level; }

#define FASTSIMD_SET_SUPPORTED_SIMD_LEVELS( LEVELS )

#define FS_FUNC_EXTERNAL( FUNC ) \
FUNC override

#define FS_INTERNAL( FUNC ) \
FUNC

#endif

