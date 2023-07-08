#pragma once
#include "example.h"

template<FastSIMD::FeatureSet SIMD>
class FastSIMD::DispatchClass<ExampleSIMD, SIMD> : public ExampleSIMD
{
    void SimpleData( const float* in, float* out, std::size_t dataSize, float multiplier, float cutoff ) override
    {
        constexpr std::size_t N = 32;

        if constexpr( !(SIMD & FastSIMD::FeatureFlag::AVX) )
        {
            auto vMultiplier = FS::f32<N>( multiplier );
            auto test = FS::NativeExec<FS::f32<N>>( FS_BIND_INTRINSIC( _mm_mul_ps ), vMultiplier, FS::Constant<float>(2,3,4,5) );

            FS::Store( out, test );
        }

                
        //auto vInt = FS::i32<N>( 1 ) + 2_i32;

        auto vMultiplier = FS::f32<N>( multiplier );
        auto vCutoff     = FS::f32<N>( cutoff );

        for( std::size_t i = 0; i < dataSize; i += N )
        {
            FS::f32<N> data = FS::Load<N>( in + i );

            data = FS::Select( data < vCutoff, data * vMultiplier, data );

            FS::Store( out + i, data );
        }
    }
};

template class FastSIMD::RegisterDispatchClass<ExampleSIMD>;