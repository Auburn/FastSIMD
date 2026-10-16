#pragma once
#include "test.h"

#include <random>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Inputs are generated and tested in chunks to keep memory use flat
constexpr size_t kChunkSize = 1 << 14;

static_assert( kChunkSize % kMaxTestLanes == 0, "Calls must not straddle input chunks" );

// Checks a test's input specs can be generated, mistakes here would silently test the wrong domain
void ValidateInputs( std::string_view testName, const TestData& test );

// A call's input values as text, one line per argument, for failure messages
std::string DescribeInputs( const TestData& test, const TestInputs& inputs, size_t inIdx );

// A lane's argument values as the oracle sees them (TestValue::Lane in test.inl), for known failure conditions
LaneArgs GetLaneArgs( const TestData& test, const TestInputs& inputs, size_t inIdx, size_t lane );

// Generates the input values for every argument of a test
// Only uses std::mt19937_64 and arithmetic without FMA contraction (see tests/CMakeLists.txt) so a seed produces the same values on any compiler
class InputGenerator
{
public:
    InputGenerator( uint64_t seed, std::string_view testName, const TestData& test );

    // Inputs below this index are combinations of special values
    size_t SpecialCount() const;

    // The kChunkSize input values per argument from chunkStart, valid until the next call
    const TestInputs& Generate( size_t chunkStart );

private:
    template<typename T>
    struct Values
    {
        std::vector<T> specials; // Empty for masks, their special values are lane patterns
        std::vector<T> chunk;
    };

    struct Arg
    {
        TestArgType type = {};
        InputSpec spec;
        size_t specialCount = 0;
        std::variant<Values<int32_t>, Values<float>, Values<double>> values;
    };

    template<typename T>
    Values<T> MakeValues( const Arg& arg ) const;

    template<typename T>
    void Fill( size_t argIdx, Values<T>& values, size_t chunkStart, std::mt19937_64& rng ) const;

    size_t SpecialDigit( size_t element, size_t argIdx ) const;
    size_t MaskPatternCount() const;
    int32_t MaskPatternLane( size_t pattern, size_t lane ) const;

    size_t mArgCount;
    size_t mLanes;
    uint64_t mSeed;
    size_t mCombinations;
    Arg mArgs[kMaxTestArgs];
    TestInputs mInputs = {};
};
