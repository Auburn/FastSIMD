#include <cstdio>
#include <iostream>

#include "../FastSIMD/FastSIMD.h"
#include "Example.h"

int main()
{
    const int size = 16;
    int data0[size];
    int data1[size];

    for ( int i = 0; i < size; i++ )
    {
        data0[i] = i;
        data1[i] = i * 2;
    }

    Example* test = FastSIMD::NewSIMDClass<Example>();

    // Force scalar
    //Example* test = FastSIMD::NewSIMDClass<Example>(FastSIMD::Level_Scalar);

    test->DoArray(&data0[0], &data1[0], size);
    
    for ( int i = 0; i < size; i++ )
    {
        printf("%d, ", data0[i]);
    }
    
    //std::cin.ignore();

	return 1;
}
