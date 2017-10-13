
#undef FASTSIMD_CLASS_DEFINITION
#undef FASTSIMD_CLASS_INIT
#undef FS_FUNC_EXTERNAL  
#undef FS_INTERNAL

#if !defined(FASTSIMD_INCLUDE_CHECK) || !defined(FS_SIMD_CLASS)

#define FASTSIMD_CLASS_DEFINITION(_CLASS) \
class _CLASS

#define FASTSIMD_CLASS_INIT()

#define FS_FUNC_EXTERNAL(_FUNC) \
virtual _FUNC = 0

#define FS_INTERNAL(_FUNC)

#ifdef FS_SIMD_CLASS
#define FASTSIMD_INCLUDE_CHECK
#endif

#else

#undef FASTSIMD_INCLUDE_CHECK

#define FS_CLASS(_CLASS) _CLASS ## _SIMD

#define FASTSIMD_CLASS_DEFINITION(_CLASS) \
template<typename FS_CLASS_T> class FS_CLASS(_CLASS) : public _CLASS

#define FASTSIMD_CLASS_INIT()             \
typedef typename FS_CLASS_T FS;           \
typedef typename FS::float32_v float32_v; \
typedef typename FS::int32_v   int32_v;   \

#define FS_FUNC_EXTERNAL(_FUNC) \
inline _FUNC

#define FS_INTERNAL(_FUNC) \
_FUNC

#endif

