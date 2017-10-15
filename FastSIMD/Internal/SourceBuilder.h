#pragma once
#include "FunctionList.h"

#define FASTSIMD_BUILD_CLASS( CLASS ) \
template FS_CLASS( CLASS ) <FS_SIMD_CLASS>;

#define FASTSIMD_INCLUDE_SOURCE

#include "../FastSIMD_BuildList.h"
