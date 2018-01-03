#include <cstdio>
#include <iostream>

#include "../FastSIMD/FastSIMD.h"
#include "Example.h"
#include "../FastSIMD/Internal/SSE.h"

int main()
{
    const int size = 16;
    int data0[size];
    int data1[size];

    for (int i = 0; i < size; i++)
    {
        data0[i] = i;
        data1[i] = i * 2;
    }

    // Will get an instance of the class using the highest SIMD level that is compiled and supported by the class
    Example* test = FastSIMD::NewSIMDClass<Example>();

    // Max level: SSE2
    // Will get an instance of the class using the highest SIMD level that is compiled and supported by the class, up to the supplied maxSIMDLevel
    //Example* test = FastSIMD::NewSIMDClass<Example>(FastSIMD::Level_SSE2);

    // Force: SSE42
    // Will throw static assert if the class doesn't support the selected level or the level is not being compiled
    //Example* test = FastSIMD::ClassFactory<Example, FastSIMD_SSE41>::Get();

    test->DoArray( data0, data1, size );

    printf( "SIMD Level: %d\n", test->GetSIMDLevel() );

    for (int i = 0; i < size; i++)
    {
        printf( "%d, ", data0[i] );
    }

    std::cin.ignore();

    return 1;
}
