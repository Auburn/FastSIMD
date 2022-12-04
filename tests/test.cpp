#include "test.h"
#include <vector>
#include <iostream>
#include <map>
#include <memory>
#include <random>

#include <FastSIMD/test_simd_config.h>

static constexpr size_t TestCount = 4096 * 4096;

int  * rndInts;
float* rndFloats;

float GenNormalFloat( std::mt19937& gen )
{
    union
    {
        float f;
        int32_t i;
    } u;

    do
    {
        u.i = (int)gen();

    } while( !std::isnormal( u.f ) );

    return u.f;
}

void GenerateRandomValues()
{
    std::cout << "Generating random values..." << std::endl;

    rndInts = new int[TestCount + 1024];
    rndFloats = new float[TestCount + 1024];

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()

    for ( std::size_t i = 0; i < TestCount; i++ )
    {
        rndInts[i] = (int)gen();
        rndFloats[i] = GenNormalFloat( gen );
    }
}

template<size_t RegisterBytes>
struct TestRunner
{
    using TestSet = std::vector<std::pair<std::string_view, std::vector<TestData>>>;

    template<typename...>
    struct TestOrganiser
    {
        static TestCollection GetCollections()
        {
            return {};
        }
    };

    template<FastSIMD::FeatureSet HEAD, FastSIMD::FeatureSet... TAIL>
    struct TestOrganiser<FastSIMD::FeatureSetList<HEAD, TAIL...>>
    { 
        static TestCollection GetCollections()
        {
            TestCollection collections = TestOrganiser<FastSIMD::FeatureSetList<TAIL...>>::GetCollections();

            if( HEAD <= FastSIMD::DetectCpuMaxFeatureSet() )
            {
                std::cout << "Generating Tests: " << FastSIMD::GetFeatureSetString( HEAD ) << std::endl;

                std::unique_ptr<TestFastSIMD<RegisterBytes>> testSimd( FastSIMD::NewDispatchClass<TestFastSIMD<RegisterBytes>>( HEAD ) );

                TestCollection simdCollection = testSimd->RegisterTests();

                collections.insert( collections.begin(), simdCollection.begin(), simdCollection.end() );
            }

            return collections;
        }

        static TestSet GetSet()
        {
            TestCollection collections = GetCollections();
            TestSet set;
            
            for( auto& collection : collections )
            {
                std::string_view& testName = collection.first;

                auto find = std::find_if( set.begin(), set.end(), [testName]( const auto& pair ){ return pair.first == testName; } );

                if( find == set.end() )
                {
                    if( collection.second.featureSet != FastSIMD::FeatureSet::Scalar )
                    {
                        throw std::exception( "Scalar must be base test set" );
                    }

                    find = set.emplace( set.end(), testName, std::vector<TestData>{} );
                }

                find->second.emplace_back( collection.second );
            }

            return set;
        }
    };

    static bool CompareOutputs( std::string_view testName, FastSIMD::FeatureSet featureSet, TestData::ReturnType returnType, size_t outputCount, void* scalarResults, void* simdResults )
    {
        bool failed = false;

        switch( returnType )
        {
        case TestData::ReturnType::boolean:
            break;
        case TestData::ReturnType::f32:
            break;
        case TestData::ReturnType::i32:
            int* intsScalar = reinterpret_cast<int*>( scalarResults );
            int* intsSimd = reinterpret_cast<int*>( simdResults );

            for( size_t idx = 0; idx < RegisterBytes / sizeof(int); idx++ )
            {
                if( intsScalar[idx] != intsSimd[idx] )
                {
                    std::cerr << FastSIMD::GetFeatureSetString( featureSet ) << " FAILED idx " << idx << ": " << testName << " Expected \"" << intsScalar[idx] << "\" Actual \"" << intsSimd[idx] << "\"" << std::endl;
                    failed = true;
                }
            }
            break;
        }

        return !failed;
    }

    static void DoTest( std::string_view testName, std::vector<TestData>& tests )
    {
        std::cout << "Running Test: " << testName << std::endl;

        char* scalarResults = new char[RegisterBytes];
        char* simdResults   = new char[RegisterBytes];

        for( size_t idx = 0; idx < TestCount; idx += RegisterBytes / sizeof( int ) )
        {
            for( size_t testIdx = 0; testIdx < tests.size(); testIdx++ )
            {
                TestData& test = tests[testIdx];
            
                char* resultsOut = testIdx ? simdResults : scalarResults;
                memset( resultsOut, (int)testIdx, RegisterBytes );
                
                size_t outputCount = test.testFunc( resultsOut, idx, rndInts, rndFloats );

                if( testIdx )
                {
                    if( !CompareOutputs( testName, test.featureSet, test.returnType, outputCount, scalarResults, simdResults ) )
                    {
                        std::cerr << "Inputs: " << test.inputsFunc( idx, rndInts, rndFloats ) << std::endl;
                        std::cin.ignore();
                    }
                }
            }
        }
        
        delete[] scalarResults;
        delete[] simdResults;
    }

    static void Run()
    {
        std::cout << "Starting Tests Register Size: " << RegisterBytes * 8 << " (" << RegisterBytes << "b)" << std::endl;

        TestSet testSet = TestOrganiser<FastSIMD::test_simd::CompiledFeatureSets>::GetSet();

        for( auto& test : testSet )
        {
            DoTest( test.first, test.second );
        }
    }
};

int main()
{
    GenerateRandomValues();

    TestRunner<128 / 8>::Run();

    return 0;
}
