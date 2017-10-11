#include <cstdio>
#include <iostream>

#include "FastSIMD/FastSIMD.h"
#include "Example.h"

int main()
{
	//Noise<FastSIMD_BASE> tInt;
	//tInt.DoStuff(3);

    int intArray[64];

    Example* t128 = FastSIMD::NewSIMDClass<Example>();
	t128->DoStuff(&intArray[0]);

    printf("%d", intArray[0]);

    //std::cin.ignore();

	return intArray[0];
}
