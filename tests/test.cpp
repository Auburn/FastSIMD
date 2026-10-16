#include "test.h"
#include "test_inputs.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <FastSIMD/simd_test_16_config.h>
#include <FastSIMD/simd_test_32_config.h>
#include <FastSIMD/simd_test_64_config.h>
#include <FastSIMD/simd_test_128_config.h>
#include <FastSIMD/simd_test_relaxed_16_config.h>
#include <FastSIMD/simd_test_relaxed_32_config.h>
#include <FastSIMD/simd_test_relaxed_64_config.h>
#include <FastSIMD/simd_test_relaxed_128_config.h>

// Every register size has a test library pair, all created for the same feature sets
using TestFeatureSets = FastSIMD::simd_test_16::CompiledFeatureSets;

static_assert( std::is_same_v<TestFeatureSets, FastSIMD::simd_test_32::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_64::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_128::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_relaxed_16::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_relaxed_32::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_relaxed_64::CompiledFeatureSets> &&
               std::is_same_v<TestFeatureSets, FastSIMD::simd_test_relaxed_128::CompiledFeatureSets>,
               "Every simd_test library must compile the same feature sets" );

struct TestOptions
{
    uint64_t seed = 24301;
    uint64_t count = 1 << 22; // Random input values per test argument, after the special value combinations
    std::string filter;       // Only run tests with names containing this
    size_t registerBytes = 0; // Only run this register size, 0 for all
    FastSIMD::FeatureSet maxFeatureSet = FastSIMD::FeatureSet::Max;
    bool stats = false;       // Print the largest relaxed differences
};

struct TestSummary
{
    size_t passed = 0;
    std::vector<std::string> knownFailures;
    std::vector<std::string> knownNotSeen;
    std::vector<std::string> failed;
};

static constexpr int kMaxFailuresPerTier = 3;

static std::string FeatureSetName( FastSIMD::FeatureSet featureSet )
{
    return FastSIMD::GetFeatureSetString( featureSet );
}

static TestTier TierOf( const TestData& test )
{
    return { test.featureSet, test.relaxed ? Tiers::Relaxed : Tiers::NonRelaxed };
}

static std::string TierName( const TestTier& tier )
{
    if( tier.kind == Tiers::Oracle )
    {
        return "SCALAR vs ORACLE";
    }
    return FeatureSetName( tier.featureSet ) + ( tier.kind == Tiers::Relaxed ? " RELAXED" : "" );
}

static std::string JoinNames( const std::vector<std::string>& names )
{
    std::string joined;
    for( size_t i = 0; i < names.size(); i++ )
    {
        joined += ( i ? ", " : "" ) + names[i];
    }
    return joined;
}

// Each test name's registrations in tier order, the first is the SCALAR non relaxed reference
using TestSet = std::vector<std::pair<std::string_view, std::vector<TestData>>>;

template<size_t RegisterBytes, bool Relaxed>
static void AddTier( TestSet& set, FastSIMD::FeatureSet featureSet )
{
    std::unique_ptr<TestFastSIMD<RegisterBytes, Relaxed>> testClass( FastSIMD::NewDispatchClass<TestFastSIMD<RegisterBytes, Relaxed>>( featureSet ) );

    if( !testClass )
    {
        throw std::runtime_error( "No test class created for " + FeatureSetName( featureSet ) );
    }

    bool isReference = featureSet == FastSIMD::FeatureSet::SCALAR && !Relaxed;

    for( auto& registered : testClass->RegisterTests() )
    {
        std::string_view testName = registered.first;
        TestData& test = registered.second;

        if( test.featureSet != featureSet || test.relaxed != Relaxed )
        {
            throw std::runtime_error( "Test class for " + FeatureSetName( featureSet ) + " registered \"" + std::string( testName ) + "\" for " + FeatureSetName( test.featureSet ) );
        }

        auto find = std::find_if( set.begin(), set.end(), [testName]( const auto& pair ) { return pair.first == testName; } );

        if( isReference )
        {
            if( find != set.end() )
            {
                throw std::runtime_error( "Multiple tests with same name: " + std::string( testName ) );
            }
            set.emplace_back( testName, std::vector<TestData>{ std::move( test ) } );
            continue;
        }

        if( find == set.end() )
        {
            throw std::runtime_error( "\"" + std::string( testName ) + "\" has no SCALAR non relaxed reference" );
        }
        if( std::any_of( find->second.begin(), find->second.end(), [&]( const TestData& other ) { return other.featureSet == featureSet && other.relaxed == Relaxed; } ) )
        {
            throw std::runtime_error( "Multiple tests with same name: " + std::string( testName ) + " (" + TierName( TierOf( test ) ) + ")" );
        }
        find->second.push_back( std::move( test ) );
    }
}

// Only creating the test classes depends on the register size
template<size_t RegisterBytes>
static TestSet CollectTests( FastSIMD::FeatureSet maxFeatureSet )
{
    TestSet set;

    for( FastSIMD::FeatureSet featureSet : TestFeatureSets::AsArray )
    {
        if( featureSet <= FastSIMD::DetectCpuMaxFeatureSet() && featureSet <= maxFeatureSet )
        {
            std::cout << "Generating Tests: " << FeatureSetName( featureSet ) << std::endl;

            AddTier<RegisterBytes, false>( set, featureSet );
            AddTier<RegisterBytes, true>( set, featureSet );
        }
    }
    return set;
}

// Every tier's results are read as the reference's types, and only the reference's settings are used, so a tier registering others is an error
static void CheckTests( std::string_view testName, const std::vector<TestData>& tests )
{
    const TestData& reference = tests[0];

    for( size_t testIdx = 1; testIdx < tests.size(); testIdx++ )
    {
        const TestData& test = tests[testIdx];

        if( test.returnType != reference.returnType || test.lanes != reference.lanes || test.argCount != reference.argCount ||
            !std::equal( test.argTypes, test.argTypes + kMaxTestArgs, reference.argTypes ) ||
            test.knownFailures.size() != reference.knownFailures.size() || test.bitExact != reference.bitExact ||
            test.relaxedTolerance != reference.relaxedTolerance || test.scalarRelaxedTolerance != reference.scalarRelaxedTolerance )
        {
            throw std::runtime_error( "Tests do not match: " + std::string( testName ) + " (" + TierName( TierOf( test ) ) + ")" );
        }
    }

    for( const KnownFailure& knownFailure : reference.knownFailures )
    {
        if( knownFailure.reason.empty() )
        {
            throw std::runtime_error( "Known failure with no reason in \"" + std::string( testName ) + "\"" );
        }
    }
}

// Value of the least significant bit, relaxed differences are counted in these
// Zero and denormals use the smallest normal rather than the denormal spacing, so relaxed results may flush denormals
template<typename T>
static T DeltaUnit( T x )
{
    x = std::fabs( x );

    if( !std::isfinite( x ) )
    {
        return T( 1 );
    }
    if( x == T( 0 ) || !std::isnormal( x ) )
    {
        return std::numeric_limits<T>::min(); // smallest positive normal number
    }

    T above = std::nextafter( x, std::numeric_limits<T>::infinity() );

    // The largest finite value has nothing finite above it, use the gap below instead
    return std::isfinite( above ) ? above - x : x - std::nextafter( x, T( 0 ) );
}

enum class LaneResult
{
    Equal, ZeroSign, Different
};

struct LaneComparison
{
    LaneResult result;
    double ulpsCovered = 0; // Relaxed difference only the ULP tolerance covered, for --stats
    double absCovered = 0;  // Relaxed difference only the absolute tolerance covered
};

template<typename T>
static LaneComparison CompareLane( const TestData& test, const TestTier& tier, T expected, T actual )
{
    if constexpr( std::is_floating_point_v<T> )
    {
        bool bitsEqual = ToBits( expected ) == ToBits( actual );
        bool bothNaN = std::isnan( expected ) && std::isnan( actual );

        if( bitsEqual || ( bothNaN && !test.bitExact ) )
        {
            return { LaneResult::Equal };
        }

        // Relaxed results are compared by value, so +0 and -0 are equal
        if( tier.kind == Tiers::Relaxed && !test.bitExact )
        {
            if( expected == actual )
            {
                return { LaneResult::Equal };
            }

            Tolerance tolerance = test.RelaxedTolerance( tier.featureSet );
            double absoluteDif = std::abs( static_cast<double>( expected ) - static_cast<double>( actual ) );
            double relativeDif = absoluteDif / DeltaUnit( expected );

            if( relativeDif <= tolerance.ulps || absoluteDif <= tolerance.absolute )
            {
                LaneComparison equal = { LaneResult::Equal };

                // Track what each tolerance had to cover on its own
                if( std::isfinite( absoluteDif ) )
                {
                    if( absoluteDif > tolerance.absolute )
                    {
                        equal.ulpsCovered = relativeDif;
                    }
                    if( relativeDif > tolerance.ulps )
                    {
                        equal.absCovered = absoluteDif;
                    }
                }
                return equal;
            }
            return { LaneResult::Different };
        }

        return { expected == 0 && actual == 0 ? LaneResult::ZeroSign : LaneResult::Different };
    }
    else
    {
        return { expected == actual ? LaneResult::Equal : LaneResult::Different };
    }
}

template<typename T>
static std::string Describe( T expected, T actual )
{
    std::ostringstream sstr;
    sstr << std::setprecision( std::is_floating_point_v<T> ? std::numeric_limits<T>::max_digits10 : 16 ) << std::boolalpha;
    sstr << "Expected \"" << expected << "\" Actual \"" << actual << "\"";

    if constexpr( std::is_floating_point_v<T> )
    {
        T absoluteDif = std::abs( expected - actual );
        sstr << " Diff \"" << absoluteDif << "\"";

        if( std::isfinite( absoluteDif ) && absoluteDif != 0 )
        {
            sstr << " (" << absoluteDif / DeltaUnit( expected ) << " ULP)";
        }
        if( ToBits( expected ) != ToBits( actual ) && ( absoluteDif == 0 || std::isnan( absoluteDif ) ) )
        {
            sstr << std::hex << " (bits 0x" << ToBits( expected ) << " vs 0x" << ToBits( actual ) << ")" << std::dec;
        }
    }
    else if constexpr( std::is_same_v<T, int32_t> )
    {
        sstr << " Diff \"" << static_cast<int64_t>( expected ) - static_cast<int64_t>( actual ) << "\"";
    }
    else if constexpr( std::is_same_v<T, int64_t> )
    {
        sstr << std::hex << " (bits 0x" << static_cast<uint64_t>( expected ) << " vs 0x" << static_cast<uint64_t>( actual ) << ")" << std::dec;
    }
    return sstr.str();
}

// A tier's run state while one test runs
struct Tier : TestTier
{
    std::string name;
    const std::function<TestFunction>& testFunc; // The tier's test, or the oracle for the oracle tier
    bool allDifferencesKnown; // A known failure covers any difference on this tier, so comparing more of its calls can't fail
    size_t compared = 0;     // Calls compared
    int failures = 0;        // Calls with differences not covered by a known failure
    int knownCalls = 0;      // Calls with only known differences
    double maxUlps = 0;      // Largest relaxed difference only the ULP tolerance covered, for --stats
    double maxAbs = 0;       // Largest relaxed difference only the absolute tolerance covered

    // A tier stops being checked after kMaxFailuresPerTier failing calls, or known failing calls if a known failure covers every difference
    bool Stopped() const
    {
        return failures >= kMaxFailuresPerTier || ( allDifferencesKnown && knownCalls >= kMaxFailuresPerTier );
    }
};

// Ordered, a call's outcome is its worst lane's
enum class CallOutcome
{
    Pass, Known, Fail
};

struct CallResult
{
    CallOutcome outcome = CallOutcome::Pass;
    std::string details; // A line for each difference no known failure covers
};

// Compares every tier of one test on every call, then reports what they found
class TestRun
{
public:
    TestRun( size_t registerBytes, std::string_view testName, const std::vector<TestData>& tests ) :
        mWidth( std::to_string( registerBytes * 8 ) + "bit " ), mTestName( testName ), mReference( tests[0] ), mKnownSeen( tests[0].knownFailures.size() )
    {
        auto addComparison = [this]( TestTier tier, const std::function<TestFunction>& testFunc )
        {
            bool allDifferencesKnown = std::any_of( mReference.knownFailures.begin(), mReference.knownFailures.end(),
                                                    [&]( const KnownFailure& knownFailure ) { return knownFailure.CoversAnyDifference() && knownFailure.Covers( tier ); } );

            mTiers.push_back( { tier, TierName( tier ), testFunc, allDifferencesKnown } );
        };

        if( mReference.oracleFunc )
        {
            addComparison( { FastSIMD::FeatureSet::SCALAR, Tiers::Oracle }, mReference.oracleFunc );
        }
        for( size_t testIdx = 1; testIdx < tests.size(); testIdx++ )
        {
            addComparison( TierOf( tests[testIdx] ), tests[testIdx].testFunc );
        }
    }

    void Run( const TestOptions& options )
    {
        switch( mReference.returnType )
        {
        case TestData::ReturnType::boolean:
            return RunCalls<bool>( options );

        case TestData::ReturnType::f32:
            return RunCalls<float>( options );

        case TestData::ReturnType::f64:
            return RunCalls<double>( options );

        case TestData::ReturnType::i32:
            return RunCalls<int32_t>( options );

        case TestData::ReturnType::i64:
            return RunCalls<int64_t>( options );
        }
    }

    void Report( const TestOptions& options, TestSummary& summary ) const
    {
        std::vector<std::string> failedTiers;
        bool comparedAny = false;

        for( const Tier& tier : mTiers )
        {
            comparedAny |= tier.compared > 0;

            if( tier.failures )
            {
                failedTiers.push_back( tier.name );
            }
            if( options.stats && tier.kind == Tiers::Relaxed && ( tier.maxUlps > 0 || tier.maxAbs > 0 ) )
            {
                Tolerance tolerance = mReference.RelaxedTolerance( tier.featureSet );
                std::cout << "  Stats " << mWidth << mTestName << " " << tier.name << ": max " << tier.maxUlps << " / " << tolerance.ulps << " ULP, "
                          << tier.maxAbs << " / " << tolerance.absolute << " abs" << std::endl;
            }
        }

        for( size_t knownIdx = 0; knownIdx < mReference.knownFailures.size(); knownIdx++ )
        {
            const KnownFailure& knownFailure = mReference.knownFailures[knownIdx];
            std::string reason( knownFailure.reason );

            if( !mKnownSeen[knownIdx].empty() )
            {
                summary.knownFailures.push_back( mWidth + std::string( mTestName ) + " (" + JoinNames( mKnownSeen[knownIdx] ) + "): " + reason );
            }
            else if( std::any_of( mTiers.begin(), mTiers.end(), [&]( const Tier& tier ) { return tier.compared && knownFailure.Covers( tier ); } ) )
            {
                summary.knownNotSeen.push_back( mWidth + std::string( mTestName ) + ": " + reason );
            }
        }

        if( !comparedAny )
        {
            std::cout << "  Warning: no tiers to compare with the reference" << std::endl;
        }

        if( !failedTiers.empty() )
        {
            summary.failed.push_back( mWidth + std::string( mTestName ) + " (" + JoinNames( failedTiers ) + ")" );
        }
        else
        {
            summary.passed++;
        }
    }

private:
    template<typename T>
    void RunCalls( const TestOptions& options )
    {
        InputGenerator generator( options.seed, mTestName, mReference );

        // No argument tests always produce the same output
        size_t inputCount = mReference.argCount ? generator.SpecialCount() + static_cast<size_t>( options.count ) : mReference.lanes;

        auto allStopped = [this]() { return std::all_of( mTiers.begin(), mTiers.end(), []( const Tier& tier ) { return tier.Stopped(); } ); };
        std::vector<char> referenceResults( kMaxTestResultBytes ), tierResults( kMaxTestResultBytes );
        const T* reference = reinterpret_cast<const T*>( referenceResults.data() );
        const T* results = reinterpret_cast<const T*>( tierResults.data() );

        for( size_t chunkStart = 0; chunkStart < inputCount && !allStopped(); chunkStart += kChunkSize )
        {
            const TestInputs& inputs = generator.Generate( chunkStart );
            size_t chunkCount = std::min( kChunkSize, inputCount - chunkStart );

            for( size_t idx = 0; idx < chunkCount; idx += mReference.lanes )
            {
                std::memset( referenceResults.data(), 0xCD, kMaxTestResultBytes );
                size_t referenceCount = mReference.testFunc( referenceResults.data(), idx, inputs );

                for( Tier& tier : mTiers )
                {
                    if( tier.Stopped() )
                    {
                        continue;
                    }

                    std::memset( tierResults.data(), 0xAB, kMaxTestResultBytes );
                    size_t outputCount = tier.testFunc( tierResults.data(), idx, inputs );

                    // The oracle is the expected value, the SCALAR reference is checked against it
                    bool oracle = tier.kind == Tiers::Oracle;
                    CallResult call = outputCount != referenceCount
                        ? CallResult{ CallOutcome::Fail, "Output count " + std::to_string( outputCount ) + ", reference " + std::to_string( referenceCount ) + "\n" }
                        : CompareCall( tier, inputs, idx, outputCount, oracle ? results : reference, oracle ? reference : results );

                    tier.compared++;

                    if( call.outcome == CallOutcome::Fail )
                    {
                        if( !tier.failures )
                        {
                            std::cerr << std::setprecision( 16 ) << "--- " << mWidth << mTestName << " " << tier.name << " FAILED ---" << std::endl;
                            std::cerr << call.details;
                            std::cerr << "Inputs (index " << chunkStart + idx << "): " << mReference.inputsFunc( idx, inputs ) << std::endl;
                        }
                        tier.failures++;
                    }
                    else if( call.outcome == CallOutcome::Known )
                    {
                        tier.knownCalls++;
                    }
                }
            }
        }
    }

    template<typename T>
    CallResult CompareCall( Tier& tier, const TestInputs& inputs, size_t inIdx, size_t outputCount, const T* expected, const T* actual )
    {
        const std::vector<KnownFailure>& knownFailures = mReference.knownFailures;
        CallResult call;

        for( size_t idx = 0; idx < outputCount; idx++ )
        {
            LaneComparison lane = CompareLane<T>( mReference, tier, expected[idx], actual[idx] );
            tier.maxUlps = std::max( tier.maxUlps, lane.ulpsCovered );
            tier.maxAbs = std::max( tier.maxAbs, lane.absCovered );

            if( lane.result == LaneResult::Equal )
            {
                continue;
            }

            LaneArgs laneArgs = mReference.laneArgsFunc( inIdx, idx, inputs );
            auto known = std::find_if( knownFailures.begin(), knownFailures.end(), [&]( const KnownFailure& knownFailure )
            {
                return knownFailure.Matches( tier, lane.result == LaneResult::ZeroSign, laneArgs );
            } );

            if( known != knownFailures.end() )
            {
                std::vector<std::string>& seen = mKnownSeen[known - knownFailures.begin()];

                if( std::find( seen.begin(), seen.end(), tier.name ) == seen.end() )
                {
                    seen.push_back( tier.name );
                }
                call.outcome = std::max( call.outcome, CallOutcome::Known );
            }
            else
            {
                call.details += "idx " + std::to_string( idx ) + ": " + std::string( mTestName ) + ( tier.kind == Tiers::Relaxed ? " RELAXED " : " " ) + Describe( expected[idx], actual[idx] ) + "\n";
                call.outcome = CallOutcome::Fail;
            }
        }
        return call;
    }

    std::string mWidth;
    std::string_view mTestName;
    const TestData& mReference;
    std::vector<Tier> mTiers;
    std::vector<std::vector<std::string>> mKnownSeen; // Tiers each known failure was seen on
};

static void DoTest( size_t registerBytes, std::string_view testName, const std::vector<TestData>& tests, const TestOptions& options, TestSummary& summary )
{
    std::cout << "Testing: " << testName << std::endl;

    CheckTests( testName, tests );
    ValidateInputs( testName, tests[0] );

    TestRun run( registerBytes, testName, tests );
    run.Run( options );
    run.Report( options, summary );
}

template<size_t RegisterBytes>
static void RunRegisterSize( const TestOptions& options, TestSummary& summary )
{
    if( options.registerBytes && options.registerBytes != RegisterBytes )
    {
        return;
    }

    std::cout << "Starting Tests - Register Size: " << RegisterBytes * 8 << " (" << RegisterBytes << "b)" << std::endl;

    for( const auto& [testName, tests] : CollectTests<RegisterBytes>( options.maxFeatureSet ) )
    {
        if( testName.find( options.filter ) != std::string_view::npos )
        {
            DoTest( RegisterBytes, testName, tests, options, summary );
        }
    }
}

template<size_t... IDXs>
static void RunTests( const TestOptions& options, TestSummary& summary, std::index_sequence<IDXs...> )
{
    ( RunRegisterSize<kTestRegisterBytes[IDXs]>( options, summary ), ... );
}

static void PrintUsage( const char* exe )
{
    std::cout << "Usage: " << exe << " [options]\n"
              << "  --seed <n|random>         Seed for random inputs (default " << TestOptions{}.seed << ")\n"
              << "  --count <n>               Random input values per test argument (default " << TestOptions{}.count << ")\n"
              << "  --filter <text>           Only run tests with names containing text\n"
              << "  --width <bytes>           Only run one register size:";
    for( size_t bytes : kTestRegisterBytes )
    {
        std::cout << " " << bytes;
    }
    std::cout << "\n  --max-feature-set <name>  Only run feature sets up to this one, to match a less capable CPU:";
    for( FastSIMD::FeatureSet featureSet : TestFeatureSets::AsArray )
    {
        std::cout << " " << FeatureSetName( featureSet );
    }
    std::cout << "\n  --stats                   Print the largest relaxed differences within tolerance" << std::endl;
}

static bool ParseUnsigned( const char* text, uint64_t& value )
{
    if( !text || !std::isdigit( static_cast<unsigned char>( text[0] ) ) )
    {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    unsigned long long parsed = std::strtoull( text, &end, 0 );

    if( errno || *end )
    {
        return false;
    }
    value = parsed;
    return true;
}

static std::string NormaliseName( std::string_view name )
{
    std::string normalised;
    for( char c : name )
    {
        if( c != '.' && c != '_' )
        {
            normalised += static_cast<char>( std::toupper( static_cast<unsigned char>( c ) ) );
        }
    }
    return normalised;
}

static bool ParseArgs( int argc, char** argv, TestOptions& options )
{
    for( int i = 1; i < argc; i++ )
    {
        std::string_view arg = argv[i];

        if( arg == "--stats" )
        {
            options.stats = true;
            continue;
        }

        const char* value = i + 1 < argc ? argv[++i] : nullptr;
        uint64_t number = 0;
        bool valid = value != nullptr;

        if( arg == "--seed" && valid )
        {
            if( std::string_view( value ) == "random" )
            {
                options.seed = std::random_device()();
            }
            else
            {
                valid = ParseUnsigned( value, options.seed );
            }
        }
        else if( arg == "--count" && valid )
        {
            // Limited so the input index can't overflow size_t on 32bit
            valid = ParseUnsigned( value, number ) && number <= ( 1ull << 30 );
            options.count = number;
        }
        else if( arg == "--filter" && valid )
        {
            options.filter = value;
        }
        else if( arg == "--width" && valid )
        {
            valid = ParseUnsigned( value, number ) &&
                    std::find( std::begin( kTestRegisterBytes ), std::end( kTestRegisterBytes ), number ) != std::end( kTestRegisterBytes );
            options.registerBytes = static_cast<size_t>( number );
        }
        else if( arg == "--max-feature-set" && valid )
        {
            const auto& featureSets = TestFeatureSets::AsArray;
            auto find = std::find_if( std::begin( featureSets ), std::end( featureSets ), [value]( FastSIMD::FeatureSet featureSet ) { return NormaliseName( FeatureSetName( featureSet ) ) == NormaliseName( value ); } );

            valid = find != std::end( featureSets );
            options.maxFeatureSet = valid ? *find : options.maxFeatureSet;
        }
        else
        {
            valid = false;
        }

        if( !valid )
        {
            std::cout << "Invalid argument: " << arg << ( value ? std::string( " " ) + value : std::string() ) << "\n" << std::endl;
            PrintUsage( argv[0] );
            return false;
        }
    }
    return true;
}

int main( int argc, char** argv )
{
    TestOptions options;

    if( !ParseArgs( argc, argv, options ) )
    {
        return 2;
    }

    std::cout << "Seed: " << options.seed << ", Random inputs per test: " << options.count << std::endl;
    std::cout << "CPU max feature set: " << FeatureSetName( FastSIMD::DetectCpuMaxFeatureSet() ) << ", Compiled:";
    for( FastSIMD::FeatureSet featureSet : TestFeatureSets::AsArray )
    {
        std::cout << " " << FeatureSetName( featureSet );
    }
    std::cout << std::endl;
    if( options.maxFeatureSet != FastSIMD::FeatureSet::Max )
    {
        std::cout << "Max feature set: " << FeatureSetName( options.maxFeatureSet ) << std::endl;
    }

    TestSummary summary;

    try
    {
        RunTests( options, summary, std::make_index_sequence<std::size( kTestRegisterBytes )>{} );
    }
    catch( const std::exception& e )
    {
        std::cerr << std::endl << "Test setup error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << std::endl << "Passed: " << summary.passed << ", Known failures: " << summary.knownFailures.size() << ", Failed: " << summary.failed.size() << std::endl;

    for( const std::string& knownFailure : summary.knownFailures )
    {
        std::cout << "  Known failure: " << knownFailure << std::endl;
    }
    for( const std::string& notSeen : summary.knownNotSeen )
    {
        std::cout << "  Known failure not seen: " << notSeen << std::endl;
    }
    for( const std::string& failed : summary.failed )
    {
        std::cout << "  FAILED: " << failed << std::endl;
    }

    if( !summary.passed && summary.failed.empty() )
    {
        std::cout << "No tests matched filter \"" << options.filter << "\"" << std::endl;
        return 1;
    }

    return summary.failed.empty() ? 0 : 1;
}
