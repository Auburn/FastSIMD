#include <cstdio>
#include <iostream>

#include "../FastSIMD/FastSIMD.h"
#include "Example.h"

int main()
{
    int intArray[64];

    Example* test = FastSIMD::NewSIMDClass<Example>();

    test->DoStuff(&intArray[0]);

    printf("%d", intArray[0]);

    //std::cin.ignore();

	return intArray[0];
}
