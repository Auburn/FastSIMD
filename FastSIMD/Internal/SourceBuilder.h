#pragma once
#include "FunctionList.h"

#define FASTSIMD_BUILD_CLASS(_CLASS) \
template FS_CLASS(_CLASS) <FS_SIMD_CLASS>;

#define FASTSIMD_INCLUDE_SOURCE

#include "../FastSIMD_BuildList.h"
