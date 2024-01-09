#pragma once
#include <FastSIMD/DispatchClass.h>
#include <cstddef>

class ExampleSIMD
{
public:
    virtual ~ExampleSIMD() = default;

    virtual void SimpleData( const float* in, float* out, std::size_t dataSize, float multiplier, float cutoff ) = 0;
};