#include "test.h"

#include <iomanip>
#include <vector>
#include <iostream>
#include <cstring>
#include <map>
#include <memory>
#include <random>
#include <string>

#include <FastSIMD/simd_test_config.h>

static constexpr size_t TestCount = 4096 * 4096;

static int  * rndInts;
static float* rndFloats;

static float GenFiniteFloat( std::mt19937& gen )
{
    union
    {
        float f;
        int32_t i;
    } u;

    do
    {
        u.i = static_cast<int>( gen() );

    } while( !std::isfinite( u.f ) );

    return u.f;
}

static void GenerateRandomValues()
{
    std::cout << "Generating random values..." << std::endl;

    rndInts = new int[TestCount + 1024];
    rndFloats = new float[TestCount + 1024];

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()

    for ( std::size_t i = 0; i < TestCount; i++ )
    {
        rndInts[i] = (int)gen();
        rndFloats[i] = GenFiniteFloat( gen );
    }

    //std::sort( rndFloats, rndFloats + TestCount + 1024, std::less() );
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
                {
                    std::unique_ptr<TestFastSIMD<RegisterBytes, true>> testSimd( FastSIMD::NewDispatchClass<TestFastSIMD<RegisterBytes, true>>( HEAD ) );

                    TestCollection simdCollection = testSimd->RegisterTests();

                    collections.insert( collections.begin(), simdCollection.begin(), simdCollection.end() );
                }
                {
                    std::unique_ptr<TestFastSIMD<RegisterBytes, false>> testSimd( FastSIMD::NewDispatchClass<TestFastSIMD<RegisterBytes, false>>( HEAD ) );

                    TestCollection simdCollection = testSimd->RegisterTests();

                    collections.insert( collections.begin(), simdCollection.begin(), simdCollection.end() );
                }
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
                    if( collection.second.featureSet != FastSIMD::FeatureSet::SCALAR )
                    {
                        throw std::runtime_error( "Scalar must be base test set" );
                    }

                    find = set.emplace( set.end(), testName, std::vector<TestData>{} );
                }

                find->second.emplace_back( collection.second );
            }

            return set;
        }
    };

    // get value of least significant bit
    static float DeltaUnit( float x )
    {
        union
        {
            float f;
            uint32_t i;
        } u;
        x = std::fabs( x );

        if( !std::isfinite( x ) )
        {
            return 1.f;
        }
        if( x == 0.f || !std::isnormal( x ) )
        {
            u.i = 0x00800000; // smallest positive normal number
            return u.f;
        }
        float x1 = x;
        u.f = x;
        u.i++;
        return u.f - x1;
    }

    template<typename T>
    static bool CompareTyped( std::string_view testName, FastSIMD::FeatureSet featureSet, float accuracy, size_t outputCount, void* scalarResults, void* simdResults )
    {
        bool success = true;

        const T* typedScalar = reinterpret_cast<T*>( scalarResults );
        const T* typedSimd = reinterpret_cast<T*>( simdResults );

        for( size_t idx = 0; idx < outputCount; idx++ )
        {
            if( typedScalar[idx] != typedSimd[idx] )
            {
                float relativeDif = 0;

                if constexpr( std::is_floating_point_v<T> )
                {
                    if( std::isnan( typedScalar[idx] ) && std::isnan( typedSimd[idx] ) )
                    {
                        continue;
                    }

                    relativeDif = std::abs( typedScalar[idx] - typedSimd[idx] ) / DeltaUnit( typedScalar[idx] );

                    if( relativeDif <= accuracy )
                    {
                        continue;
                    }
                }
                if( success )
                {
                    std::cerr << std::setprecision( 16 ) << std::boolalpha;
                    std::cerr << "--- " << FastSIMD::GetFeatureSetString( featureSet ) << " FAILED ---" << std::endl;
                }
                std::cerr << "idx " << idx << ": " << testName
                          << " Expected \"" << typedScalar[idx]
                          << "\" Actual \"" << typedSimd[idx]
                          << "\" Diff \"" << std::abs( typedScalar[idx] - typedSimd[idx] ) << "\"";

                if( relativeDif != 0.0f )
                {
                    std::cerr << " (" << relativeDif << ")";
                }
                std::cerr << std::endl;
                success = false;
            }
        }

        return success;
    }

    static bool CompareOutputs( std::string_view testName, FastSIMD::FeatureSet featureSet, TestData::ReturnType returnType, float accuracy, size_t outputCount, void* scalarResults, void* simdResults )
    {
        switch( returnType )
        {
        case TestData::ReturnType::boolean:
            return CompareTyped<bool>( testName, featureSet, accuracy, outputCount, scalarResults, simdResults );

        case TestData::ReturnType::f32:
            return CompareTyped<float>( testName, featureSet, accuracy, outputCount, scalarResults, simdResults );

        case TestData::ReturnType::i32:
            return CompareTyped<int32_t>( testName, featureSet, accuracy, outputCount, scalarResults, simdResults );
        }

        return false;
    }

    static void DoTest( std::string_view testName, std::vector<TestData>& tests )
    {
        std::cout << "Testing: " << testName << std::endl;

        char* scalarResults = new char[RegisterBytes];
        char* simdResults   = new char[RegisterBytes];

        for( size_t idx = 0; idx < TestCount; idx += RegisterBytes / sizeof( int ) )
        {
            int failed = 0;

            for( size_t testIdx = 0; testIdx < tests.size(); testIdx++ )
            {               
                TestData& test = tests[testIdx];
            
                char* resultsOut = testIdx ? simdResults : scalarResults;
                std::memset( resultsOut, (int)testIdx, RegisterBytes );
                
                size_t outputCount = test.testFunc( resultsOut, idx, rndInts, rndFloats );

                if( testIdx )
                {
                    if( test.returnType != tests[0].returnType )
                    {
                        std::cerr << "Tests do not match: " << testName; 
                        throw std::exception();
                    }
                    if( test.featureSet == FastSIMD::FeatureSet::SCALAR && !test.relaxed )
                    {
                        std::cerr << "Multiple tests with same name: " << testName; 
                        throw std::exception();
                    }

                    std::string testNameRelaxed = testName.data();
                    float accuracy = 0;

                    if( test.relaxed )
                    {
                        testNameRelaxed += " RELAXED";
                        accuracy = test.relaxedAccuracy;
                    }

                    if( !CompareOutputs( testNameRelaxed, test.featureSet, test.returnType, accuracy, outputCount, scalarResults, simdResults ) )
                    {
                        std::cerr << "Inputs: " << tests[0].inputsFunc( idx, rndInts, rndFloats ) << std::endl;
                        failed++;
                    }
                }
            }

            if( failed >= 3 )
            {
                std::cerr << "Skipping test, fail limit reached" << std::endl;
                break;
            }
        }
        
        delete[] scalarResults;
        delete[] simdResults;
    }

    static void Run()
    {
        std::cout << "Starting Tests - Register Size: " << RegisterBytes * 8 << " (" << RegisterBytes << "b)" << std::endl;

        TestSet testSet = TestOrganiser<FastSIMD::test_simd::CompiledFeatureSets>::GetSet();

        for( auto& test : testSet )
        {
            //if( test.first.find( "sqrt" ) != std::string_view::npos )
            {
                DoTest( test.first, test.second );
            }
        }

        std::cout << "Testing Complete!" << std::endl;
    }
};

int main()
{
    GenerateRandomValues();

    TestRunner<kTestBytes>::Run();

    return 0;
}
