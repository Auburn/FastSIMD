#pragma once
#include "Utility/ArchDetect.h"
#include "Utility/FeatureEnums.h"

namespace FastSIMD
{
    static_assert( FASTSIMD_DEFAULT_FEATURE_VALUE() <= FASTSIMD_MAX_FEATURE_VALUE(),
                   "Default feature set must be <= to max feature set" );

    template<FeatureSet SIMD = FastSIMD::FeatureSet::FASTSIMD_DEFAULT_FEATURE_SET>
    static constexpr FeatureSet FeatureSetDefault()
    {
        return SIMD;
    }
} // namespace FastSIMD


#include "ToolSet/Generic/Scalar.h"

#if FASTSIMD_CURRENT_ARCH_IS( X86 )
#include "ToolSet/x86/SSE.h"
#endif

#include "ToolSet/Generic/Functions.h"
