#pragma once
#include "Utility/FeatureEnums.h"

#include <cstddef>

namespace FastSIMD
{        
    using MemoryAllocator = void* (*)( std::size_t size, std::size_t align );

    // Creates T for the highest compiled feature set <= maxFeatureSet that the CPU supports, nullptr if there is none
    template<typename T>
    FASTSIMD_API T* NewDispatchClass( FeatureSet maxFeatureSet = FeatureSet::Max, MemoryAllocator allocator = nullptr );
}
