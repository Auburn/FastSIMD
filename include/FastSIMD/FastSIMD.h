#pragma once
#include "Utility/FeatureEnums.h"

#if defined(__arm__) || defined(__aarch64__)
#define FASTSIMD_ARM
#else
#define FASTSIMD_X86
#endif

#if INTPTR_MAX == INT64_MAX
#define FASTSIMD_64BIT
#endif

