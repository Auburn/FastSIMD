#pragma once

#include "../simdtypes/TypeList.h"

#define FASTSIMD_BUILD_CLASS(_CLASS) \
template FS_CLASS(_CLASS) <FS_SIMD_CLASS>;

#define FASTSIMD_INCLUDE_EXT FASTSIMD_INCLUDE_SOURCE_EXT

#include "../FastSIMD_BuildList.h"
