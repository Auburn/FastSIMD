#pragma once
#include <vector>
#include <functional>

class SIMDUnitTest
{
public:

    static void RunAll();

    SIMDUnitTest( std::function<void( void* )> func )
    {
        tests.emplace_back( func );
    }
    
private:
    static std::vector<std::function<void(void*)> > tests;
    
};
