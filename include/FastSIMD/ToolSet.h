#pragma once
#include "Utility/ArchDetect.h"
#include "Utility/FeatureEnums.h"

#if INTPTR_MAX == INT64_MAX
#define FASTSIMD_64BIT
#endif

#include "ToolSet/Generic/Scalar.h"

#if FASTSIMD_CURRENT_ARCH_IS( X86 )
#include "ToolSet/x86/SSE.h"
#endif

#include "ToolSet/Generic/Functions.h"
