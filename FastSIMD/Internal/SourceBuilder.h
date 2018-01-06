#pragma once
#include "FunctionList.h"
#include "FastSIMD.h"

#define FASTSIMD_BUILD_CLASS( CLASS ) \
template<typename SIMD_T>\
struct FastSIMD::ClassFactory<FS_ENABLE_IF( (CLASS::Supported_SIMD_Levels & SIMD_T::SIMD_Level & FastSIMD::COMPILED_SIMD_LEVELS) == 0, CLASS ), SIMD_T>\
{\
static CLASS* Get()\
{\
return nullptr; \
}\
};\
template<typename SIMD_T>\
struct FastSIMD::ClassFactory<FS_ENABLE_IF( (CLASS::Supported_SIMD_Levels & SIMD_T::SIMD_Level & FastSIMD::COMPILED_SIMD_LEVELS) != 0, CLASS ), SIMD_T>\
{\
static CLASS* Get()\
{\
return new FS_CLASS(CLASS)<SIMD_T>; \
}\
};\
template FastSIMD::ClassFactory <CLASS, FS_SIMD_CLASS>;


#include "../FastSIMD_BuildList.h"
