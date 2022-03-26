#pragma once
#include "example.h"

#include <FastSIMD/ToolSet/DispatchClassImpl.h>

template<FastSIMD::FeatureSet SIMD>
class FastSIMD::DispatchClass<ExampleSIMD, SIMD> : public ExampleSIMD
{
    void SimpleData( const float* in, float* out, std::size_t dataSize, float multiplier, float cutoff ) override
    {
        if constexpr( SIMD == FastSIMD::FeatureSet::SSE41 )
        {
            //static_assert( !(SIMD == FastSIMD::FeatureSet::SSE41));
        }

        constexpr std::size_t N = 16;

        auto vMultiplier = FS::f32<N>( multiplier );
        auto vCutoff     = FS::f32<N>( cutoff );

        for( std::size_t i = 0; i < dataSize; i += N )
        {
            FS::f32<N> data = FS::Load<FS::f32<N>>( in + i );

            data = FS::Select( data < vCutoff, data * vMultiplier, data );

            FS::Store( out + i, data );
        }
    }
};

template class FastSIMD::RegisterDispatchClass<ExampleSIMD>;