#include "example.h"
#include <vector>
#include <iostream>

int main()
{
    FastSIMD::FeatureSet featureSet = FastSIMD::DetectCpuMaxFeatureSet();
    std::cout << FastSIMD::GetFeatureSetString( featureSet ) << std::endl;

    std::vector<float> data;
    for( int i = 0; i < 32; i++ )
    {
        data.push_back( (float)i );
    }
    std::vector<float> out( data.size() );

    ExampleSIMD* simd = FastSIMD::NewDispatchClass<ExampleSIMD>();

    simd->SimpleData( data.data(), out.data(), data.size(), 10, 17 );
    
    for( std::size_t i = 0; i < data.size(); i++ )
    {
        std::cout << data[i] << "\t: " << out[i] << std::endl;
    }

    return 0;
}
