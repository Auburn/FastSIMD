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

    template<auto = 0, bool RELAXED = FASTSIMD_IS_RELAXED>
    static constexpr bool IsRelaxed()
    {
        return RELAXED;
    }
} // namespace FastSIMD

#include "ToolSet/Generic/Functions.h"

#include "ToolSet/Generic/Scalar.h"

#if FASTSIMD_CURRENT_ARCH_IS( X86 )
#include "ToolSet/x86/x86.h"
#elif FASTSIMD_CURRENT_ARCH_IS( ARM )
#include "ToolSet/ARM/ARM.h"
#elif FASTSIMD_CURRENT_ARCH_IS( WASM )
#include "ToolSet/WASM/WASM.h"
#endif

