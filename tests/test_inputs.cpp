#include "test_inputs.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

// Calls func with a value of the type an argument's inputs are stored as
template<typename FUNC>
static void WithStoredType( TestElement element, FUNC&& func )
{
    switch( element )
    {
    case TestElement::Int32:
        return func( int32_t() );
    case TestElement::Float:
        return func( float() );
    case TestElement::Double:
        return func( double() );
    }
}

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

void ValidateInputs( std::string_view testName, const TestData& test )
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

        if( argIdx >= test.argCount )
        {
            if( !spec.IsDefault() )
            {
                fail( argIdx, "test has fewer arguments than input specs" );
            }
            continue;
        }

        TestArgType type = test.argTypes[argIdx];
        if( type.mask && !spec.IsDefault() )
        {
            fail( argIdx, "mask arguments are always generated from lane patterns, they take no input spec" );
        }
        int maxBits = type.element == TestElement::Float ? FloatTraits<float>::kSignificandBits : type.element == TestElement::Double ? FloatTraits<double>::kSignificandBits : 0;
        if( spec.significandBits < 0 || spec.significandBits > maxBits )
        {
            fail( argIdx, maxBits ? "significand bits must be in [1, 24] for floats, [1, 53] for doubles" : "significand bits only apply to floats and doubles" );
        }
        for( double special : spec.specials )
        {
            if( std::isnan( special ) )
            {
                fail( argIdx, "special values can't be NaN, the default spec includes NaNs" );
            }
        }
        if( ( spec.kind == InputSpec::Kind::NoNaN || spec.kind == InputSpec::Kind::Finite ) && type.element == TestElement::Int32 )
        {
            fail( argIdx, "NoNaN and Finite only apply to floats and doubles" );
        }
        if( spec.kind != InputSpec::Kind::Range && spec.kind != InputSpec::Kind::Magnitude )
        {
            continue;
        }
        if( !std::isfinite( spec.lo ) || !std::isfinite( spec.hi ) || spec.lo > spec.hi )
        {
            fail( argIdx, "range bounds must be finite with lo <= hi" );
        }
        if( spec.kind == InputSpec::Kind::Magnitude && spec.lo <= 0 )
        {
            fail( argIdx, "magnitude lo must be > 0" );
        }
        if( type.element == TestElement::Float && ( std::abs( spec.lo ) > floatMax || std::abs( spec.hi ) > floatMax ) )
        {
            fail( argIdx, "range bounds must be within float range" );
        }
        if( type.element == TestElement::Int32 &&
            ( std::ceil( spec.lo ) > std::floor( spec.hi ) || std::ceil( spec.lo ) < -2147483648.0 || std::floor( spec.hi ) > 2147483647.0 ) )
        {
            fail( argIdx, "int range must contain an integer and be within int32 range" );
        }
    }
}

std::string DescribeInputs( const TestData& test, const TestInputs& inputs, size_t inIdx )
{
    if( !test.argCount )
    {
        return "No Inputs";
    }

    std::ostringstream description;
    description << std::setprecision( 17 ) << std::boolalpha;

    for( size_t argIdx = 0; argIdx < test.argCount; argIdx++ )
    {
        TestArgType type = test.argTypes[argIdx];
        bool isRegister = type.lanes > 1;

        // Registers list their lanes, mask lanes are true or false
        WithStoredType( type.element, [&]( auto stored )
        {
            const auto* values = inputs.Get<decltype( stored )>( argIdx ) + inIdx;

            description << "\n" << ( isRegister ? "[" : "" );
            for( size_t lane = 0; lane < type.lanes; lane++ )
            {
                description << ( lane ? ", " : "" );
                if( type.mask )
                {
                    description << ( values[lane] != 0 );
                }
                else
                {
                    description << values[lane];
                }
            }
            description << ( isRegister ? "]" : "" );
        } );
    }
    return description.str();
}

LaneArgs GetLaneArgs( const TestData& test, const TestInputs& inputs, size_t inIdx, size_t lane )
{
    LaneArgs args = {};

    for( size_t argIdx = 0; argIdx < test.argCount; argIdx++ )
    {
        TestArgType type = test.argTypes[argIdx];
        size_t idx = inIdx + ( type.lanes > 1 ? lane : 0 );

        WithStoredType( type.element, [&]( auto stored )
        {
            auto value = inputs.Get<decltype( stored )>( argIdx )[idx];
            if( type.mask )
            {
                args.Set( argIdx, value != 0 );
            }
            else
            {
                args.Set( argIdx, value );
            }
        } );
    }
    return args;
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
        return true;
    case InputSpec::Kind::NoNaN:
        return !std::isnan( value );
    case InputSpec::Kind::Finite:
        return std::isfinite( value );
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
    const float denormMax = FromBits<float>( 0x007FFFFF );

    std::vector<float> candidates = {
        0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 1.5f, -1.5f, 2.5f, -2.5f,
        0.49999997f, -0.49999997f, 0.99999994f, -0.99999994f, 1.0000001f, -1.0000001f,
        floatMin, -floatMin, denormMin, -denormMin, denormMax, -denormMax,
        std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
        8388607.5f, -8388607.5f, 8388608.0f, -8388608.0f, 16777216.0f, -16777216.0f,
        2147483520.0f, -2147483520.0f, 2147483648.0f, -2147483648.0f,
        3.14159274f, -3.14159274f, 1.e-10f, 1.e+10f,
    };
    if( spec.kind != InputSpec::Kind::Finite )
    {
        candidates.insert( candidates.end(), { inf, -inf } );
    }
    if( spec.kind == InputSpec::Kind::Any )
    {
        // Quiet, negative, signalling and payload NaNs
        for( uint32_t bits : { 0x7FC00000u, 0xFFC00000u, 0x7F800001u, 0xFF800001u, 0x7FFFFFFFu, 0xFFC00001u } )
        {
            candidates.push_back( FromBits<float>( bits ) );
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
    if( spec.kind != InputSpec::Kind::Finite )
    {
        candidates.insert( candidates.end(), { inf, -inf } );
    }
    if( spec.kind == InputSpec::Kind::Any )
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
    case InputSpec::Kind::NoNaN:
    case InputSpec::Kind::Finite:
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
    case InputSpec::Kind::NoNaN:
    case InputSpec::Kind::Finite:
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
        case InputSpec::Kind::NoNaN:
        case InputSpec::Kind::Finite:
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
        case InputSpec::Kind::NoNaN:
        case InputSpec::Kind::Finite:
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
    case InputSpec::Kind::NoNaN:
    case InputSpec::Kind::Finite:
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

template<typename T>
static std::vector<T> Specials( const InputSpec& spec )
{
    if constexpr( std::is_same_v<T, int32_t> )
    {
        return IntSpecials( spec );
    }
    else
    {
        return FloatSpecials<T>( spec );
    }
}

template<typename T>
static T RandomValue( const InputSpec& spec, std::mt19937_64& rng )
{
    if constexpr( std::is_same_v<T, int32_t> )
    {
        return RandomInt( spec, rng );
    }
    else
    {
        return RandomFloat<T>( spec, rng );
    }
}

InputGenerator::InputGenerator( uint64_t seed, std::string_view testName, const TestData& test ) :
    mArgCount( test.argCount ), mLanes( test.lanes ), mSeed( seed ^ HashName( testName ) ), mCombinations( test.argCount ? 1 : 0 )
{
    for( size_t argIdx = 0; argIdx < mArgCount; argIdx++ )
    {
        Arg& arg = mArgs[argIdx];
        arg.type = test.argTypes[argIdx];
        arg.spec = test.inputs[argIdx];

        WithStoredType( arg.type.element, [&]( auto stored ) { arg.values = MakeValues<decltype( stored )>( arg ); } );

        arg.specialCount = arg.type.mask ? MaskPatternCount() : std::visit( []( const auto& values ) { return values.specials.size(); }, arg.values );
        mCombinations *= arg.specialCount;
    }
}

size_t InputGenerator::SpecialCount() const
{
    return mCombinations * mLanes;
}

const TestInputs& InputGenerator::Generate( size_t chunkStart )
{
    for( size_t argIdx = 0; argIdx < mArgCount; argIdx++ )
    {
        uint64_t state = mSeed + chunkStart * kMaxTestArgs + argIdx;
        std::mt19937_64 rng( SplitMix64( state ) );

        std::visit( [&]( auto& values )
        {
            Fill( argIdx, values, chunkStart, rng );
            mInputs.values[argIdx] = values.chunk.data();
        }, mArgs[argIdx].values );
    }
    return mInputs;
}

template<typename T>
InputGenerator::Values<T> InputGenerator::MakeValues( const Arg& arg ) const
{
    Values<T> values;
    if( !arg.type.mask )
    {
        values.specials = Specials<T>( arg.spec );
    }
    values.chunk.resize( kChunkSize );
    return values;
}

template<typename T>
void InputGenerator::Fill( size_t argIdx, Values<T>& values, size_t chunkStart, std::mt19937_64& rng ) const
{
    const Arg& arg = mArgs[argIdx];

    for( size_t i = 0; i < kChunkSize; i++ )
    {
        size_t element = chunkStart + i;
        bool special = element < SpecialCount();

        if( arg.type.mask )
        {
            int32_t lane = special ? MaskPatternLane( SpecialDigit( element, argIdx ), element % mLanes ) : static_cast<int32_t>( rng() & 1 );
            values.chunk[i] = static_cast<T>( lane );
        }
        else
        {
            values.chunk[i] = special ? values.specials[SpecialDigit( element, argIdx )] : RandomValue<T>( arg.spec, rng );
        }
    }
}

// Value arguments use combination (call + lane), putting every combination in every lane and in lane 0 for scalar arguments
// Mask arguments use combination (call) so a whole mask pattern lands in one call
size_t InputGenerator::SpecialDigit( size_t element, size_t argIdx ) const
{
    size_t call = element / mLanes;
    size_t combination = mArgs[argIdx].type.mask ? call : call + element % mLanes;
    combination %= mCombinations;

    for( size_t i = 0; i < argIdx; i++ )
    {
        combination /= mArgs[i].specialCount;
    }
    return combination % mArgs[argIdx].specialCount;
}

// All clear, all set, each single lane set, each single lane clear, alternating, and each half set
size_t InputGenerator::MaskPatternCount() const
{
    return 2 + mLanes * 2 + 4;
}

int32_t InputGenerator::MaskPatternLane( size_t pattern, size_t lane ) const
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
