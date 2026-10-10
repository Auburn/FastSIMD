#include "test.h"

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

// Inputs are generated and tested in chunks to keep memory use flat
static constexpr size_t kChunkSize = 1 << 14;
static constexpr int kMaxFailuresPerTier = 3;

static_assert( kChunkSize % kMaxTestLanes == 0, "Calls must not straddle input chunks" );

static uint64_t SplitMix64( uint64_t& state )
{
    uint64_t z = ( state += 0x9E3779B97F4A7C15ull );
    z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
    z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
    return z ^ ( z >> 31 );
}

// FNV-1a, stable across compilers unlike std::hash
static uint64_t HashName( std::string_view name )
{
    uint64_t hash = 0xCBF29CE484222325ull;
    for( char c : name )
    {
        hash = ( hash ^ static_cast<uint8_t>( c ) ) * 0x100000001B3ull;
    }
    return hash;
}

template<typename T>
struct FloatTraits;

template<>
struct FloatTraits<float>
{
    using Bits = uint32_t;
    static constexpr int kSignificandBits = 24;
};

template<>
struct FloatTraits<double>
{
    using Bits = uint64_t;
    static constexpr int kSignificandBits = 53;
};

template<typename T>
static typename FloatTraits<T>::Bits ToBits( T f )
{
    typename FloatTraits<T>::Bits bits;
    std::memcpy( &bits, &f, sizeof( f ) );
    return bits;
}

template<typename T>
static T FromBits( typename FloatTraits<T>::Bits bits )
{
    T f;
    std::memcpy( &f, &bits, sizeof( f ) );
    return f;
}

static float BitsToFloat( uint32_t bits ) { return FromBits<float>( bits ); }

static std::string FeatureSetName( FastSIMD::FeatureSet featureSet )
{
    return FastSIMD::GetFeatureSetString( featureSet );
}

template<typename>
struct FeatureSetsOf;

template<FastSIMD::FeatureSet... SETS>
struct FeatureSetsOf<FastSIMD::FeatureSetList<0, SETS...>>
{
    static std::vector<FastSIMD::FeatureSet> Get() { return { SETS... }; }
};

static std::vector<FastSIMD::FeatureSet> CompiledFeatureSets()
{
    return FeatureSetsOf<TestFeatureSets>::Get();
}

// Checks a test's input specs can be generated, mistakes here would silently test the wrong domain
static void ValidateInputs( std::string_view testName, const TestData& test )
{
    auto fail = [&]( size_t argIdx, const char* problem )
    {
        std::string message = "Invalid input spec for \"" + std::string( testName ) + "\" argument " + std::to_string( argIdx ) + ": " + problem;
        std::cerr << message << std::endl;
        throw std::runtime_error( message );
    };

    constexpr double floatMax = std::numeric_limits<float>::max();

    for( size_t argIdx = 0; argIdx < kMaxTestArgs; argIdx++ )
    {
        const InputSpec& spec = test.inputs[argIdx];
        bool isDefault = spec.kind == InputSpec::Kind::Any && !spec.inf && !spec.nan && spec.significandBits == 0 && spec.specials.empty();

        if( argIdx >= test.argCount )
        {
            if( !isDefault )
            {
                fail( argIdx, "test has fewer arguments than input specs" );
            }
            continue;
        }

        if( test.argTypes[argIdx] == TestArgType::Mask && !isDefault )
        {
            fail( argIdx, "mask arguments are always generated from lane patterns, they take no input spec" );
        }
        TestArgType type = test.argTypes[argIdx];
        int maxBits = type == TestArgType::Float ? 24 : type == TestArgType::Double ? 53 : 0;
        if( spec.significandBits < 0 || spec.significandBits > maxBits )
        {
            fail( argIdx, maxBits ? "significand bits must be in [1, 24] for floats, [1, 53] for doubles" : "significand bits only apply to floats and doubles" );
        }
        for( double special : spec.specials )
        {
            if( std::isnan( special ) )
            {
                fail( argIdx, "special values can't be NaN, use AnyNonFinite" );
            }
        }
        if( spec.kind == InputSpec::Kind::Any )
        {
            continue;
        }
        if( spec.inf || spec.nan )
        {
            fail( argIdx, "inf and NaN are only supported by Any specs" );
        }
        if( !std::isfinite( spec.lo ) || !std::isfinite( spec.hi ) || spec.lo > spec.hi )
        {
            fail( argIdx, "range bounds must be finite with lo <= hi" );
        }
        if( spec.kind == InputSpec::Kind::Magnitude && spec.lo <= 0 )
        {
            fail( argIdx, "magnitude lo must be > 0" );
        }
        if( test.argTypes[argIdx] == TestArgType::Float && ( std::abs( spec.lo ) > floatMax || std::abs( spec.hi ) > floatMax ) )
        {
            fail( argIdx, "range bounds must be within float range" );
        }
        if( test.argTypes[argIdx] == TestArgType::Int &&
            ( std::ceil( spec.lo ) > std::floor( spec.hi ) || std::ceil( spec.lo ) < -2147483648.0 || std::floor( spec.hi ) > 2147483647.0 ) )
        {
            fail( argIdx, "int range must contain an integer and be within int32 range" );
        }
    }
}

// Generates the input values for every argument of a test
// Only uses std::mt19937_64 and arithmetic without FMA contraction (see tests/CMakeLists.txt) so a seed produces the same values on any compiler
class InputGenerator
{
public:
    InputGenerator( uint64_t seed, std::string_view testName, const TestData& test, size_t lanes ) :
        mArgCount( test.argCount ), mLanes( lanes )
    {
        mSeed = seed ^ HashName( testName );
        mCombinations = mArgCount ? 1 : 0;

        for( size_t argIdx = 0; argIdx < mArgCount; argIdx++ )
        {
            Arg& arg = mArgs[argIdx];
            arg.type = test.argTypes[argIdx];
            arg.spec = test.inputs[argIdx];

            switch( arg.type )
            {
            case TestArgType::Float:
                arg.floatSpecials = FloatSpecials<float>( arg.spec );
                arg.specialCount = arg.floatSpecials.size();
                arg.floats.resize( kChunkSize );
                break;
            case TestArgType::Double:
                arg.doubleSpecials = FloatSpecials<double>( arg.spec );
                arg.specialCount = arg.doubleSpecials.size();
                arg.doubles.resize( kChunkSize );
                break;
            case TestArgType::Int:
                arg.intSpecials = IntSpecials( arg.spec );
                arg.specialCount = arg.intSpecials.size();
                arg.ints.resize( kChunkSize );
                break;
            case TestArgType::Mask:
                arg.specialCount = MaskPatternCount();
                arg.ints.resize( kChunkSize );
                arg.maskFloats.resize( kChunkSize );
                arg.maskDoubles.resize( kChunkSize );
                break;
            }

            mCombinations *= arg.specialCount;
        }
    }

    // Inputs below this index are combinations of special values
    size_t SpecialCount() const
    {
        return mCombinations * mLanes;
    }

    void Generate( size_t chunkStart, TestInputs& inputs )
    {
        inputs = {};

        for( size_t argIdx = 0; argIdx < mArgCount; argIdx++ )
        {
            Arg& arg = mArgs[argIdx];
            uint64_t state = mSeed + chunkStart * kMaxTestArgs + argIdx;
            std::mt19937_64 rng( SplitMix64( state ) );

            for( size_t i = 0; i < kChunkSize; i++ )
            {
                size_t element = chunkStart + i;
                bool special = element < SpecialCount();

                switch( arg.type )
                {
                case TestArgType::Float:
                    arg.floats[i] = special ? arg.floatSpecials[SpecialDigit( element, argIdx )] : RandomFloat<float>( arg.spec, rng );
                    break;
                case TestArgType::Double:
                    arg.doubles[i] = special ? arg.doubleSpecials[SpecialDigit( element, argIdx )] : RandomFloat<double>( arg.spec, rng );
                    break;
                case TestArgType::Int:
                    arg.ints[i] = special ? arg.intSpecials[SpecialDigit( element, argIdx )] : RandomInt( arg.spec, rng );
                    break;
                case TestArgType::Mask:
                    arg.ints[i] = special ? MaskPatternLane( SpecialDigit( element, argIdx ), element % mLanes ) : static_cast<int32_t>( rng() & 1 );
                    arg.maskFloats[i] = static_cast<float>( arg.ints[i] );
                    arg.maskDoubles[i] = static_cast<double>( arg.ints[i] );
                    break;
                }
            }

            switch( arg.type )
            {
            case TestArgType::Float:
                inputs.floats[argIdx] = arg.floats.data();
                break;
            case TestArgType::Double:
                inputs.doubles[argIdx] = arg.doubles.data();
                break;
            case TestArgType::Int:
                inputs.ints[argIdx] = arg.ints.data();
                break;
            case TestArgType::Mask:
                inputs.masks[argIdx] = arg.ints.data();
                inputs.maskFloats[argIdx] = arg.maskFloats.data();
                inputs.maskDoubles[argIdx] = arg.maskDoubles.data();
                break;
            }
        }
    }

private:
    struct Arg
    {
        TestArgType type = TestArgType::Float;
        InputSpec spec;
        std::vector<float> floatSpecials;
        std::vector<double> doubleSpecials;
        std::vector<int32_t> intSpecials;
        size_t specialCount = 0;
        std::vector<float> floats;
        std::vector<double> doubles;
        std::vector<int32_t> ints; // Ints, or 0/1 mask lanes
        std::vector<float> maskFloats;
        std::vector<double> maskDoubles;
    };

    // Value arguments use combination (call + lane), putting every combination in every lane and in lane 0 for scalar arguments
    // Mask arguments use combination (call) so a whole mask pattern lands in one call
    size_t SpecialDigit( size_t element, size_t argIdx ) const
    {
        size_t call = element / mLanes;
        size_t combination = mArgs[argIdx].type == TestArgType::Mask ? call : call + element % mLanes;
        combination %= mCombinations;

        for( size_t i = 0; i < argIdx; i++ )
        {
            combination /= mArgs[i].specialCount;
        }
        return combination % mArgs[argIdx].specialCount;
    }

    // All clear, all set, each single lane set, each single lane clear, alternating, and each half set
    size_t MaskPatternCount() const
    {
        return 2 + mLanes * 2 + 4;
    }

    int32_t MaskPatternLane( size_t pattern, size_t lane ) const
    {
        if( pattern < 2 )
        {
            return static_cast<int32_t>( pattern );
        }
        pattern -= 2;
        if( pattern < mLanes )
        {
            return lane == pattern;
        }
        pattern -= mLanes;
        if( pattern < mLanes )
        {
            return lane != pattern;
        }
        pattern -= mLanes;

        switch( pattern )
        {
        case 0: return ( lane & 1 ) == 0;
        case 1: return ( lane & 1 ) == 1;
        case 2: return lane < mLanes / 2;
        default: return lane >= mLanes / 2;
        }
    }

    template<typename T>
    static T RangeLo( const InputSpec& spec ) { return static_cast<T>( spec.lo ); }
    template<typename T>
    static T RangeHi( const InputSpec& spec ) { return static_cast<T>( spec.hi ); }

    // Int bounds are the integers inside [lo, hi], ValidateInputs checks there is at least one
    static int64_t IntLo( const InputSpec& spec ) { return static_cast<int64_t>( std::ceil( spec.lo ) ); }
    static int64_t IntHi( const InputSpec& spec ) { return static_cast<int64_t>( std::floor( spec.hi ) ); }

    // Drops the low significand bits, finite values only
    template<typename T>
    static T Truncate( T value, int significandBits )
    {
        constexpr int kBits = FloatTraits<T>::kSignificandBits;
        if( significandBits == 0 || significandBits >= kBits || !std::isfinite( value ) )
        {
            return value;
        }
        using Bits = typename FloatTraits<T>::Bits;
        Bits lowBits = ( Bits( 1 ) << ( kBits - significandBits ) ) - 1;
        return FromBits<T>( ToBits( value ) & ~lowBits );
    }

    template<typename T>
    static bool InFloatDomain( const InputSpec& spec, T value )
    {
        switch( spec.kind )
        {
        case InputSpec::Kind::Any:
            return ( spec.nan || !std::isnan( value ) ) && ( spec.inf || !std::isinf( value ) );
        case InputSpec::Kind::Range:
            return value >= RangeLo<T>( spec ) && value <= RangeHi<T>( spec );
        case InputSpec::Kind::Magnitude:
            return std::abs( value ) >= RangeLo<T>( spec ) && std::abs( value ) <= RangeHi<T>( spec );
        }
        return false;
    }

    // Values of every magnitude that commonly break SIMD code
    static std::vector<float> AnySpecials( const InputSpec& spec, float )
    {
        constexpr float inf = std::numeric_limits<float>::infinity();
        constexpr float floatMin = std::numeric_limits<float>::min();
        constexpr float denormMin = std::numeric_limits<float>::denorm_min();
        const float denormMax = BitsToFloat( 0x007FFFFF );

        std::vector<float> candidates = {
            0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 1.5f, -1.5f, 2.5f, -2.5f,
            0.49999997f, -0.49999997f, 0.99999994f, -0.99999994f, 1.0000001f, -1.0000001f,
            floatMin, -floatMin, denormMin, -denormMin, denormMax, -denormMax,
            std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
            8388607.5f, -8388607.5f, 8388608.0f, -8388608.0f, 16777216.0f, -16777216.0f,
            2147483520.0f, -2147483520.0f, 2147483648.0f, -2147483648.0f,
            3.14159274f, -3.14159274f, 1.e-10f, 1.e+10f,
        };
        if( spec.inf )
        {
            candidates.insert( candidates.end(), { inf, -inf } );
        }
        if( spec.nan )
        {
            // Quiet, negative, signalling and payload NaNs
            for( uint32_t bits : { 0x7FC00000u, 0xFFC00000u, 0x7F800001u, 0xFF800001u, 0x7FFFFFFFu, 0xFFC00001u } )
            {
                candidates.push_back( BitsToFloat( bits ) );
            }
        }
        return candidates;
    }

    static std::vector<double> AnySpecials( const InputSpec& spec, double )
    {
        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double doubleMin = std::numeric_limits<double>::min();
        constexpr double denormMin = std::numeric_limits<double>::denorm_min();
        const double denormMax = FromBits<double>( 0x000FFFFFFFFFFFFFull );
        constexpr double floatMax = std::numeric_limits<float>::max();
        constexpr double floatMin = std::numeric_limits<float>::min();
        constexpr double floatDenormMin = std::numeric_limits<float>::denorm_min();

        std::vector<double> candidates = {
            0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -1.5, 2.5, -2.5,
            std::nextafter( 0.5, 0.0 ), -std::nextafter( 0.5, 0.0 ), std::nextafter( 1.0, 0.0 ), -std::nextafter( 1.0, 0.0 ),
            std::nextafter( 1.0, 2.0 ), -std::nextafter( 1.0, 2.0 ),
            doubleMin, -doubleMin, denormMin, -denormMin, denormMax, -denormMax,
            std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
            // Integer precision limits, and the int32 convert range
            4503599627370495.5, -4503599627370495.5, 4503599627370496.0, -4503599627370496.0, 9007199254740992.0, -9007199254740992.0,
            2147483647.0, -2147483648.0, 2147483647.5, -2147483648.5, 2147483648.0, -2147483649.0,
            // Narrowing to float: overflow, float denormals, and rounding ties
            floatMax, -floatMax, 3.4028235677973366e+38, -3.4028235677973366e+38, floatMin, -floatMin, floatDenormMin, -floatDenormMin,
            floatDenormMin / 2, 1.0 + 0x1p-24, -( 1.0 + 0x1p-24 ), 1.0 + 0x1.8p-23, 1.0 + 0x1p-23 + 0x1p-25,
            3.141592653589793, -3.141592653589793, 1.e-10, 1.e+10, 1.e-300, 1.e+300,
        };
        if( spec.inf )
        {
            candidates.insert( candidates.end(), { inf, -inf } );
        }
        if( spec.nan )
        {
            // Quiet, negative, signalling and payload NaNs, including payloads narrowing to float drops
            for( uint64_t bits : { 0x7FF8000000000000ull, 0xFFF8000000000000ull, 0x7FF0000000000001ull, 0xFFF0000000000001ull,
                                   0x7FFFFFFFFFFFFFFFull, 0xFFF8000000000001ull, 0x7FF4000000000000ull } )
            {
                candidates.push_back( FromBits<double>( bits ) );
            }
        }
        return candidates;
    }

    template<typename T>
    static std::vector<T> FloatSpecials( const InputSpec& spec )
    {
        constexpr T inf = std::numeric_limits<T>::infinity();
        constexpr T typeMin = std::numeric_limits<T>::min();
        constexpr T denormMin = std::numeric_limits<T>::denorm_min();
        const T denormMax = std::nextafter( typeMin, T( 0 ) );
        const T belowHalf = std::nextafter( T( 0.5 ), T( 0 ) );
        const T belowOne = std::nextafter( T( 1 ), T( 0 ) );
        const T aboveOne = std::nextafter( T( 1 ), T( 2 ) );
        std::vector<T> candidates;

        switch( spec.kind )
        {
        case InputSpec::Kind::Any:
            candidates = AnySpecials( spec, T() );
            break;

        case InputSpec::Kind::Range:
            candidates = { RangeLo<T>( spec ), RangeHi<T>( spec ), std::nextafter( RangeLo<T>( spec ), inf ), std::nextafter( RangeHi<T>( spec ), -inf ) };
            for( T v : { T( 0 ), T( 0.5 ), T( 1 ), T( 1.5 ), T( 2.5 ), belowHalf, belowOne, aboveOne, typeMin, denormMin, denormMax, T( 1.e-10 ), T( 1.e-30 ) } )
            {
                candidates.insert( candidates.end(), { v, -v } );
            }
            break;

        case InputSpec::Kind::Magnitude:
            for( T v : { RangeLo<T>( spec ), RangeHi<T>( spec ), std::nextafter( RangeLo<T>( spec ), inf ), std::nextafter( RangeHi<T>( spec ), -inf ),
                         T( 0.5 ), T( 1 ), T( 2 ), belowOne, aboveOne, typeMin, denormMin, denormMax } )
            {
                candidates.insert( candidates.end(), { v, -v } );
            }
            break;
        }

        for( double special : spec.specials )
        {
            T v = static_cast<T>( special );
            candidates.insert( candidates.end(), { v, std::nextafter( v, inf ), std::nextafter( v, -inf ) } );
        }

        std::vector<T> specials;
        for( T candidate : candidates )
        {
            T v = Truncate( candidate, spec.significandBits );

            // Compare bits to keep both 0 and -0, and each NaN
            bool duplicate = std::any_of( specials.begin(), specials.end(), [v]( T s ) { return ToBits( s ) == ToBits( v ); } );

            if( InFloatDomain( spec, v ) && !duplicate )
            {
                specials.push_back( v );
            }
        }
        if( specials.empty() )
        {
            throw std::runtime_error( "Input spec has no special values in its domain" );
        }
        return specials;
    }

    static std::vector<int32_t> IntSpecials( const InputSpec& spec )
    {
        constexpr int32_t intMax = std::numeric_limits<int32_t>::max();
        constexpr int32_t intMin = std::numeric_limits<int32_t>::min();
        std::vector<int64_t> candidates;

        switch( spec.kind )
        {
        case InputSpec::Kind::Any:
            candidates = {
                0, 1, -1, 2, -2, 31, 32, 33, -31, -32,
                intMax, intMin, intMax - 1, intMin + 1,
                0x7F800000, static_cast<int32_t>( 0xFF800000 ), 0x7FC00000, 0x7F800001, // inf, -inf, NaN, signalling NaN float bits
                0x3F800000, 0x00800000, 0x0000FFFF, 0x00010000, 0x55555555, static_cast<int32_t>( 0xAAAAAAAA ),
            };
            break;
        case InputSpec::Kind::Range:
            candidates = { IntLo( spec ), IntHi( spec ), IntLo( spec ) + 1, IntHi( spec ) - 1, 0, 1, -1 };
            break;
        case InputSpec::Kind::Magnitude:
            for( int64_t v : { IntLo( spec ), IntHi( spec ), IntLo( spec ) + 1, IntHi( spec ) - 1, int64_t( 1 ) } )
            {
                candidates.insert( candidates.end(), { v, -v } );
            }
            break;
        }

        for( double special : spec.specials )
        {
            candidates.push_back( static_cast<int64_t>( std::round( std::min( std::max( special, -2147483648.0 ), 2147483647.0 ) ) ) );
        }

        std::vector<int32_t> specials;
        for( int64_t v : candidates )
        {
            bool inDomain = true;
            switch( spec.kind )
            {
            case InputSpec::Kind::Any:
                inDomain = v >= intMin && v <= intMax;
                break;
            case InputSpec::Kind::Range:
                inDomain = v >= IntLo( spec ) && v <= IntHi( spec );
                break;
            case InputSpec::Kind::Magnitude:
                inDomain = std::abs( v ) >= IntLo( spec ) && std::abs( v ) <= IntHi( spec );
                break;
            }

            if( inDomain && std::find( specials.begin(), specials.end(), v ) == specials.end() )
            {
                specials.push_back( static_cast<int32_t>( v ) );
            }
        }
        return specials;
    }

    // Uniform [0, 1)
    static double RandomUnit( std::mt19937_64& rng )
    {
        return static_cast<double>( rng() >> 11 ) * ( 1.0 / 9007199254740992.0 );
    }

    // Log scaled magnitude within [lo, hi], lo > 0: a uniform power of 2 binade, then uniform within its part of [lo, hi]
    template<typename T>
    static T RandomMagnitude( double lo, double hi, std::mt19937_64& rng )
    {
        int loExp, hiExp;
        std::frexp( lo, &loExp );
        std::frexp( hi, &hiExp );

        int exponent = loExp + static_cast<int>( rng() % static_cast<uint64_t>( hiExp - loExp + 1 ) );
        double binadeLo = std::max( lo, std::ldexp( 0.5, exponent ) );
        double binadeHi = std::min( hi, std::ldexp( 1.0, exponent ) );
        T value = static_cast<T>( binadeLo + ( binadeHi - binadeLo ) * RandomUnit( rng ) );

        return std::min( std::max( value, static_cast<T>( lo ) ), static_cast<T>( hi ) );
    }

    template<typename T>
    static T RandomFloat( const InputSpec& spec, std::mt19937_64& rng )
    {
        constexpr double denormMin = std::numeric_limits<T>::denorm_min();

        for( ;; )
        {
            T value = 0;

            switch( spec.kind )
            {
            case InputSpec::Kind::Any:
                value = FromBits<T>( static_cast<typename FloatTraits<T>::Bits>( rng() ) );
                break;

            case InputSpec::Kind::Range:
                if( rng() & 1 )
                {
                    value = static_cast<T>( spec.lo + ( spec.hi - spec.lo ) * RandomUnit( rng ) );
                }
                else
                {
                    // Log scaled half covers small magnitudes, including denormals, that uniform values almost never hit
                    bool positive = spec.lo >= 0 || ( spec.hi > 0 && ( rng() & 1 ) );
                    double maxMag = positive ? spec.hi : -spec.lo;
                    double minMag = positive ? std::max( spec.lo, 0.0 ) : std::max( -spec.hi, 0.0 );
                    value = RandomMagnitude<T>( std::max( minMag, denormMin ), std::max( maxMag, denormMin ), rng );
                    value = positive ? value : -value;
                }
                break;

            case InputSpec::Kind::Magnitude:
                value = RandomMagnitude<T>( spec.lo, spec.hi, rng );
                value = ( rng() & 1 ) ? -value : value;
                break;
            }

            value = Truncate( value, spec.significandBits );

            if( InFloatDomain( spec, value ) )
            {
                return value;
            }
        }
    }

    static int32_t RandomInt( const InputSpec& spec, std::mt19937_64& rng )
    {
        switch( spec.kind )
        {
        case InputSpec::Kind::Any:
            return static_cast<int32_t>( rng() );

        case InputSpec::Kind::Range:
            return static_cast<int32_t>( IntLo( spec ) + static_cast<int64_t>( rng() % static_cast<uint64_t>( IntHi( spec ) - IntLo( spec ) + 1 ) ) );

        case InputSpec::Kind::Magnitude:
        {
            int64_t value = IntLo( spec ) + static_cast<int64_t>( rng() % static_cast<uint64_t>( IntHi( spec ) - IntLo( spec ) + 1 ) );
            return static_cast<int32_t>( ( rng() & 1 ) ? -value : value );
        }
        }
        return 0;
    }

    size_t mArgCount;
    size_t mLanes;
    uint64_t mSeed;
    size_t mCombinations;
    Arg mArgs[kMaxTestArgs];
};

enum class LaneResult
{
    Equal, ZeroSign, Different
};

struct TierResult
{
    TestTier tier;
    std::string name;
    const TestData* data = nullptr;
    size_t compared = 0;     // Calls compared
    int failures = 0;        // Calls with differences not covered by a known failure
    int knownCalls = 0;      // Calls with only known differences
    double maxUlps = 0;      // Largest relaxed difference only the ULP tolerance covered, for --stats
    double maxAbs = 0;       // Largest relaxed difference only the absolute tolerance covered
};

template<size_t RegisterBytes>
struct TestRunner
{
    using TestSet = std::vector<std::pair<std::string_view, std::vector<TestData>>>;

    template<typename...>
    struct TestOrganiser
    {
        static TestCollection GetCollections( FastSIMD::FeatureSet )
        {
            return {};
        }
    };

    template<FastSIMD::FeatureSet HEAD, FastSIMD::FeatureSet... TAIL>
    struct TestOrganiser<FastSIMD::FeatureSetList<0, HEAD, TAIL...>>
    {
        template<bool RELAXED>
        static TestCollection RegisterTests()
        {
            std::unique_ptr<TestFastSIMD<RegisterBytes, RELAXED>> testSimd( FastSIMD::NewDispatchClass<TestFastSIMD<RegisterBytes, RELAXED>>( HEAD ) );

            if( !testSimd )
            {
                throw std::runtime_error( "No test class created for " + FeatureSetName( HEAD ) );
            }

            TestCollection collection = testSimd->RegisterTests();

            for( auto& test : collection )
            {
                if( test.second.featureSet != HEAD || test.second.relaxed != RELAXED )
                {
                    throw std::runtime_error( "Test class for " + FeatureSetName( HEAD ) + " registered \"" + std::string( test.first ) + "\" for " + FeatureSetName( test.second.featureSet ) );
                }
            }
            return collection;
        }

        static TestCollection GetCollections( FastSIMD::FeatureSet maxFeatureSet )
        {
            TestCollection collections = TestOrganiser<FastSIMD::FeatureSetList<0, TAIL...>>::GetCollections( maxFeatureSet );

            if( HEAD <= FastSIMD::DetectCpuMaxFeatureSet() && HEAD <= maxFeatureSet )
            {
                std::cout << "Generating Tests: " << FeatureSetName( HEAD ) << std::endl;

                TestCollection relaxed = RegisterTests<true>();
                collections.insert( collections.begin(), relaxed.begin(), relaxed.end() );

                TestCollection nonRelaxed = RegisterTests<false>();
                collections.insert( collections.begin(), nonRelaxed.begin(), nonRelaxed.end() );
            }

            return collections;
        }

        static TestSet GetSet( FastSIMD::FeatureSet maxFeatureSet )
        {
            TestCollection collections = GetCollections( maxFeatureSet );
            TestSet set;

            for( auto& collection : collections )
            {
                std::string_view& testName = collection.first;

                auto find = std::find_if( set.begin(), set.end(), [testName]( const auto& pair ){ return pair.first == testName; } );

                if( find == set.end() )
                {
                    if( collection.second.featureSet != FastSIMD::FeatureSet::SCALAR || collection.second.relaxed )
                    {
                        throw std::runtime_error( "\"" + std::string( testName ) + "\" has no SCALAR non relaxed reference" );
                    }

                    find = set.emplace( set.end(), testName, std::vector<TestData>{} );
                }

                find->second.emplace_back( collection.second );
            }

            return set;
        }
    };

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

    template<typename T>
    static LaneResult CompareLane( const TestData& test, bool relaxed, T expected, T actual, TierResult& result )
    {
        if constexpr( std::is_floating_point_v<T> )
        {
            bool bitsEqual = ToBits( expected ) == ToBits( actual );
            bool bothNaN = std::isnan( expected ) && std::isnan( actual );

            if( bitsEqual || ( bothNaN && !test.bitExact ) )
            {
                return LaneResult::Equal;
            }

            // Relaxed results are compared by value, so +0 and -0 are equal
            if( relaxed && !test.bitExact )
            {
                if( expected == actual )
                {
                    return LaneResult::Equal;
                }

                auto [allowedUlps, allowedAbs] = test.RelaxedTolerance();
                double absoluteDif = std::abs( static_cast<double>( expected ) - static_cast<double>( actual ) );
                double relativeDif = absoluteDif / DeltaUnit( expected );

                if( relativeDif <= allowedUlps || absoluteDif <= allowedAbs )
                {
                    // Track what each tolerance had to cover on its own
                    if( std::isfinite( absoluteDif ) )
                    {
                        if( absoluteDif > allowedAbs )
                        {
                            result.maxUlps = std::max( result.maxUlps, relativeDif );
                        }
                        if( relativeDif > allowedUlps )
                        {
                            result.maxAbs = std::max( result.maxAbs, absoluteDif );
                        }
                    }
                    return LaneResult::Equal;
                }
                return LaneResult::Different;
            }

            return expected == 0 && actual == 0 ? LaneResult::ZeroSign : LaneResult::Different;
        }
        else
        {
            return expected == actual ? LaneResult::Equal : LaneResult::Different;
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

    // Compares one call's outputs, returns true if the tier failed. Known differences mark their entry in knownSeen and set sawKnown
    template<typename T>
    static bool CompareCall( std::string_view testName, const TestData& reference, TierResult& result, size_t outputCount, const TestInputs& inputs, size_t inIdx,
                             const void* expectedResults, const void* actualResults, std::vector<std::vector<std::string>>& knownSeen, std::string& details, bool& sawKnown )
    {
        const T* expected = reinterpret_cast<const T*>( expectedResults );
        const T* actual = reinterpret_cast<const T*>( actualResults );
        const TestData& test = result.data ? *result.data : reference;
        const auto& knownFailures = reference.knownFailures;
        bool failed = false;

        for( size_t idx = 0; idx < outputCount; idx++ )
        {
            LaneResult lane = CompareLane<T>( test, result.tier.relaxed, expected[idx], actual[idx], result );

            if( lane == LaneResult::Equal )
            {
                continue;
            }

            // Lane inputs for known failure conditions
            TestData::LaneArgs laneArgs = {};
            for( size_t argIdx = 0; argIdx < reference.argCount; argIdx++ )
            {
                if( reference.argTypes[argIdx] == TestArgType::Float )
                {
                    laneArgs.floats[argIdx] = inputs.floats[argIdx][inIdx + idx];
                }
                else if( reference.argTypes[argIdx] == TestArgType::Double )
                {
                    laneArgs.doubles[argIdx] = inputs.doubles[argIdx][inIdx + idx];
                }
            }

            bool known = false;
            for( size_t knownIdx = 0; knownIdx < knownFailures.size(); knownIdx++ )
            {
                const TestData::KnownFailureInfo& knownFailure = knownFailures[knownIdx];

                if( knownFailure.Covers( result.tier ) && ( !knownFailure.zeroSignOnly || lane == LaneResult::ZeroSign ) &&
                    ( !knownFailure.inputCondition || knownFailure.inputCondition( laneArgs ) ) )
                {
                    known = true;
                    sawKnown = true;

                    if( std::find( knownSeen[knownIdx].begin(), knownSeen[knownIdx].end(), result.name ) == knownSeen[knownIdx].end() )
                    {
                        knownSeen[knownIdx].push_back( result.name );
                    }
                    break;
                }
            }

            if( !known )
            {
                details += "idx " + std::to_string( idx ) + ": " + std::string( testName ) + ( result.tier.relaxed ? " RELAXED " : " " ) + Describe( expected[idx], actual[idx] ) + "\n";
                failed = true;
            }
        }
        return failed;
    }

    static bool CompareOutputs( std::string_view testName, const TestData& reference, TierResult& result, size_t outputCount, const TestInputs& inputs, size_t inIdx,
                                const void* expected, const void* actual, std::vector<std::vector<std::string>>& knownSeen, std::string& details, bool& sawKnown )
    {
        switch( reference.returnType )
        {
        case TestData::ReturnType::boolean:
            return CompareCall<bool>( testName, reference, result, outputCount, inputs, inIdx, expected, actual, knownSeen, details, sawKnown );

        case TestData::ReturnType::f32:
            return CompareCall<float>( testName, reference, result, outputCount, inputs, inIdx, expected, actual, knownSeen, details, sawKnown );

        case TestData::ReturnType::f64:
            return CompareCall<double>( testName, reference, result, outputCount, inputs, inIdx, expected, actual, knownSeen, details, sawKnown );

        case TestData::ReturnType::i32:
            return CompareCall<int32_t>( testName, reference, result, outputCount, inputs, inIdx, expected, actual, knownSeen, details, sawKnown );

        case TestData::ReturnType::i64:
            return CompareCall<int64_t>( testName, reference, result, outputCount, inputs, inIdx, expected, actual, knownSeen, details, sawKnown );
        }

        return true;
    }

    static std::string TierName( const TestTier& tier )
    {
        if( tier.oracle )
        {
            return "SCALAR vs ORACLE";
        }
        return FeatureSetName( tier.featureSet ) + ( tier.relaxed ? " RELAXED" : "" );
    }

    static void CheckTestsMatch( std::string_view testName, const std::vector<TestData>& tests )
    {
        const TestData& reference = tests[0];

        for( size_t testIdx = 1; testIdx < tests.size(); testIdx++ )
        {
            const TestData& test = tests[testIdx];

            if( test.featureSet == FastSIMD::FeatureSet::SCALAR && !test.relaxed )
            {
                throw std::runtime_error( "Multiple tests with same name: " + std::string( testName ) );
            }
            if( test.returnType != reference.returnType || test.lanes != reference.lanes || test.argCount != reference.argCount || test.knownFailures.size() != reference.knownFailures.size() ||
                !std::equal( test.argTypes, test.argTypes + kMaxTestArgs, reference.argTypes ) )
            {
                throw std::runtime_error( "Tests do not match: " + std::string( testName ) + " (" + TierName( { test.featureSet, test.relaxed, false } ) + ")" );
            }
        }
    }

    static void DoTest( std::string_view testName, std::vector<TestData>& tests, const TestOptions& options, TestSummary& summary )
    {
        std::cout << "Testing: " << testName << std::endl;

        const TestData& reference = tests[0];
        CheckTestsMatch( testName, tests );
        ValidateInputs( testName, reference );

        InputGenerator generator( options.seed, testName, reference, reference.lanes );
        TestInputs inputs;

        // No argument tests always produce the same output
        size_t inputCount = reference.argCount ? generator.SpecialCount() + static_cast<size_t>( options.count ) : reference.lanes;

        std::vector<TierResult> tiers;
        if( reference.oracleFunc )
        {
            TierResult oracle;
            oracle.tier = { FastSIMD::FeatureSet::SCALAR, false, true };
            oracle.name = TierName( oracle.tier );
            tiers.push_back( oracle );
        }
        for( size_t testIdx = 1; testIdx < tests.size(); testIdx++ )
        {
            TierResult tier;
            tier.tier = { tests[testIdx].featureSet, tests[testIdx].relaxed, false };
            tier.name = TierName( tier.tier );
            tier.data = &tests[testIdx];
            tiers.push_back( tier );
        }

        // A tier stops being checked after kMaxFailuresPerTier failing calls, or known failing calls if a known failure covers every difference
        auto stopped = [&]( const TierResult& tier )
        {
            if( tier.failures >= kMaxFailuresPerTier )
            {
                return true;
            }
            bool coversAll = std::any_of( reference.knownFailures.begin(), reference.knownFailures.end(),
                                          [&]( const TestData::KnownFailureInfo& known ) { return known.CoversAnyDifference() && known.Covers( tier.tier ); } );
            return coversAll && tier.knownCalls >= kMaxFailuresPerTier;
        };

        std::vector<std::vector<std::string>> knownSeen( reference.knownFailures.size() );
        std::vector<char> referenceResults( kMaxTestResultBytes ), testResults( kMaxTestResultBytes );

        for( size_t chunkStart = 0; chunkStart < inputCount && !tiers.empty(); chunkStart += kChunkSize )
        {
            if( std::all_of( tiers.begin(), tiers.end(), stopped ) )
            {
                break;
            }

            generator.Generate( chunkStart, inputs );
            size_t chunkCount = std::min( kChunkSize, inputCount - chunkStart );

            for( size_t idx = 0; idx < chunkCount; idx += reference.lanes )
            {
                std::memset( referenceResults.data(), 0xCD, kMaxTestResultBytes );
                size_t referenceCount = reference.testFunc( referenceResults.data(), idx, inputs );

                for( TierResult& tier : tiers )
                {
                    if( stopped( tier ) )
                    {
                        continue;
                    }

                    std::memset( testResults.data(), 0xAB, kMaxTestResultBytes );
                    size_t outputCount = tier.data ? tier.data->testFunc( testResults.data(), idx, inputs ) : reference.oracleFunc( testResults.data(), idx, inputs );

                    std::string details;
                    bool failed;
                    bool sawKnown = false;

                    if( outputCount != referenceCount )
                    {
                        details = "Output count " + std::to_string( outputCount ) + ", reference " + std::to_string( referenceCount ) + "\n";
                        failed = true;
                    }
                    else if( tier.data )
                    {
                        failed = CompareOutputs( testName, reference, tier, outputCount, inputs, idx, referenceResults.data(), testResults.data(), knownSeen, details, sawKnown );
                    }
                    else
                    {
                        // The oracle is the expected value, the SCALAR reference is checked against it
                        failed = CompareOutputs( testName, reference, tier, outputCount, inputs, idx, testResults.data(), referenceResults.data(), knownSeen, details, sawKnown );
                    }

                    tier.compared++;

                    if( failed )
                    {
                        if( !tier.failures )
                        {
                            std::cerr << std::setprecision( 16 ) << "--- " << RegisterBytes * 8 << "bit " << testName << " " << tier.name << " FAILED ---" << std::endl;
                            std::cerr << details;
                            std::cerr << "Inputs (index " << chunkStart + idx << "): " << reference.inputsFunc( idx, inputs ) << std::endl;
                        }
                        tier.failures++;
                    }
                    else if( sawKnown )
                    {
                        tier.knownCalls++;
                    }
                }
            }
        }

        std::string width = std::to_string( RegisterBytes * 8 ) + "bit ";
        std::vector<std::string> failedTiers;
        bool comparedAny = false;

        for( const TierResult& tier : tiers )
        {
            comparedAny |= tier.compared > 0;

            if( tier.failures )
            {
                failedTiers.push_back( tier.name );
            }
            if( options.stats && tier.data && tier.tier.relaxed && ( tier.maxUlps > 0 || tier.maxAbs > 0 ) )
            {
                auto [allowedUlps, allowedAbs] = tier.data->RelaxedTolerance();
                std::cout << "  Stats " << width << testName << " " << tier.name << ": max " << tier.maxUlps << " / " << allowedUlps << " ULP, "
                          << tier.maxAbs << " / " << allowedAbs << " abs" << std::endl;
            }
        }

        auto join = []( const std::vector<std::string>& names )
        {
            std::string joined;
            for( size_t i = 0; i < names.size(); i++ )
            {
                joined += ( i ? ", " : "" ) + names[i];
            }
            return joined;
        };

        for( size_t knownIdx = 0; knownIdx < reference.knownFailures.size(); knownIdx++ )
        {
            const TestData::KnownFailureInfo& knownFailure = reference.knownFailures[knownIdx];
            std::string reason = knownFailure.reason ? knownFailure.reason : "(no reason given)";

            if( !knownSeen[knownIdx].empty() )
            {
                summary.knownFailures.push_back( width + std::string( testName ) + " (" + join( knownSeen[knownIdx] ) + "): " + reason );
            }
            else if( std::any_of( tiers.begin(), tiers.end(), [&]( const TierResult& tier ) { return tier.compared && knownFailure.Covers( tier.tier ); } ) )
            {
                summary.knownNotSeen.push_back( width + std::string( testName ) + ": " + reason );
            }
        }

        if( !comparedAny )
        {
            std::cout << "  Warning: no tiers to compare with the reference" << std::endl;
        }

        if( !failedTiers.empty() )
        {
            summary.failed.push_back( width + std::string( testName ) + " (" + join( failedTiers ) + ")" );
        }
        else
        {
            summary.passed++;
        }
    }

    static void Run( const TestOptions& options, TestSummary& summary )
    {
        if( options.registerBytes && options.registerBytes != RegisterBytes )
        {
            return;
        }

        std::cout << "Starting Tests - Register Size: " << RegisterBytes * 8 << " (" << RegisterBytes << "b)" << std::endl;

        TestSet testSet = TestOrganiser<TestFeatureSets>::GetSet( options.maxFeatureSet );

        for( auto& test : testSet )
        {
            if( test.first.find( options.filter ) != std::string_view::npos )
            {
                DoTest( test.first, test.second, options, summary );
            }
        }
    }
};

template<size_t... IDXs>
static void RunTests( const TestOptions& options, TestSummary& summary, std::index_sequence<IDXs...> )
{
    ( TestRunner<kTestRegisterBytes[IDXs]>::Run( options, summary ), ... );
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
    for( FastSIMD::FeatureSet featureSet : CompiledFeatureSets() )
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
            std::vector<FastSIMD::FeatureSet> featureSets = CompiledFeatureSets();
            auto find = std::find_if( featureSets.begin(), featureSets.end(), [value]( FastSIMD::FeatureSet featureSet ) { return NormaliseName( FeatureSetName( featureSet ) ) == NormaliseName( value ); } );

            valid = find != featureSets.end();
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
    for( FastSIMD::FeatureSet featureSet : CompiledFeatureSets() )
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
