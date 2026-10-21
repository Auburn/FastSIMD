#pragma once
#include "Utility/FeatureEnums.h"

#include <cstddef>

namespace FastSIMD
{        
    using MemoryAllocator = void* (*)( std::size_t size, std::size_t align );

    // Creates T for the highest compiled feature set <= maxFeatureSet (FeatureSet::Max: the CPU's max feature set)
    // Returns nullptr if the CPU doesn't support maxFeatureSet, or no compiled feature set is <= maxFeatureSet
    template<typename T>
    FASTSIMD_API T* NewDispatchClass( FeatureSet maxFeatureSet = FeatureSet::Max, MemoryAllocator allocator = nullptr );
}
