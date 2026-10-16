#pragma once
#include <FastSIMD/DispatchClass.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <string>

// Register sizes the tests are compiled for, each is a pair of dispatch libraries in tests/CMakeLists.txt
constexpr size_t kTestRegisterBytes[] = { 16, 32, 64, 128 };
// Lanes per call is set by a test's widest lane type, 32bit lanes give the most
constexpr size_t kMaxTestLanes = 128 / sizeof( int32_t );
// Converts keep the lane count, so 32bit lanes widened to 64bit need twice the register size
constexpr size_t kMaxTestResultBytes = kMaxTestLanes * sizeof( double );

constexpr size_t kMaxTestArgs = 3;

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
typename FloatTraits<T>::Bits ToBits( T f )
{
    typename FloatTraits<T>::Bits bits;
    std::memcpy( &bits, &f, sizeof( f ) );
    return bits;
}

template<typename T>
T FromBits( typename FloatTraits<T>::Bits bits )
{
    T f;
    std::memcpy( &f, &bits, sizeof( f ) );
    return f;
}

// The type an argument's input values are stored as, one buffer per argument
enum class TestElement : uint8_t
{
    Int32, Float, Double
};

struct TestArgType
{
    TestElement element;
    bool mask; // Lanes are 0 or 1 in the element type, from lane patterns rather than an input spec, compared != 0 to make the mask

    bool operator==( const TestArgType& other ) const { return element == other.element && mask == other.mask; }
};

// One pointer per argument to its values for the current chunk, indexed by input index and stored as the argument's TestElement
struct TestInputs
{
    const void* values[kMaxTestArgs];

    template<typename T>
    const T* Get( size_t argIdx ) const
    {
        return static_cast<const T*>( values[argIdx] );
    }
};

// A lane's input values, for known failure conditions. Arguments of other types are 0
struct LaneArgs
{
    int32_t ints[kMaxTestArgs]; // Masks are 0 or 1
    float floats[kMaxTestArgs];
    double doubles[kMaxTestArgs];

    void Set( size_t argIdx, int32_t value ) { ints[argIdx] = value; }
    void Set( size_t argIdx, bool value ) { ints[argIdx] = value; }
    void Set( size_t argIdx, float value ) { floats[argIdx] = value; }
    void Set( size_t argIdx, double value ) { doubles[argIdx] = value; }
};

// How input values are generated for a test argument, the default spec is every value of the argument's type
// Every test first runs all combinations of the special values for its inputs, then random values
struct InputSpec
{
    enum class Kind
    {
        Any,       // Values of every magnitude, +-inf and NaNs
        NoNaN,     // Values of every magnitude and +-inf
        Finite,    // Values of every magnitude
        Range,     // Values within [lo, hi], half uniform, half log scaled
        Magnitude, // Values with magnitude within [lo, hi] log scaled, either sign, lo > 0
    };

    Kind kind = Kind::Any;
    double lo = 0;
    double hi = 0;
    int significandBits = 0; // Floats are truncated to this many significand bits, 0 for all of them
    std::vector<double> specials; // Extra special values, only used when inside the spec's domain

    static InputSpec NoNaN() { InputSpec spec; spec.kind = Kind::NoNaN; return spec; }
    static InputSpec Finite() { InputSpec spec; spec.kind = Kind::Finite; return spec; }
    static InputSpec Range( double lo, double hi ) { InputSpec spec; spec.kind = Kind::Range; spec.lo = lo; spec.hi = hi; return spec; }
    static InputSpec Magnitude( double lo, double hi ) { InputSpec spec; spec.kind = Kind::Magnitude; spec.lo = lo; spec.hi = hi; return spec; }

    InputSpec Bits( int bits ) const { InputSpec spec = *this; spec.significandBits = bits; return spec; }

    // Adds each value and the floats (or doubles) either side of it
    InputSpec Specials( std::initializer_list<double> values ) const
    {
        InputSpec spec = *this;
        spec.specials.insert( spec.specials.end(), values.begin(), values.end() );
        return spec;
    }

    bool IsDefault() const
    {
        return kind == Kind::Any && significandBits == 0 && specials.empty();
    }
};

using TestFunction = size_t ( void* outResult, size_t inIdx, const TestInputs& inputs );
using InputsFunction = std::string ( size_t inIdx, const TestInputs& inputs );
using LaneArgsFunction = LaneArgs ( size_t inIdx, size_t lane, const TestInputs& inputs );

// Kinds of tier, each tier is one kind and known failures are scoped to a set of them
enum class Tiers : uint8_t
{
    None = 0,
    NonRelaxed = 1 << 0, // A non relaxed feature set against the SCALAR reference, bit for bit
    Relaxed = 1 << 1,    // A relaxed feature set against the SCALAR reference, within the test's tolerance
    Oracle = 1 << 2,     // The SCALAR reference against the test's oracle
    All = NonRelaxed | Relaxed, // Every tier compared with the reference, not the oracle
};

constexpr Tiers operator|( Tiers a, Tiers b )
{
    return static_cast<Tiers>( static_cast<uint8_t>( a ) | static_cast<uint8_t>( b ) );
}

constexpr bool operator&( Tiers a, Tiers b )
{
    return static_cast<uint8_t>( a ) & static_cast<uint8_t>( b );
}

// What a result is checked against: a feature set against the SCALAR reference, or the reference against the oracle
struct TestTier
{
    FastSIMD::FeatureSet featureSet;
    Tiers kind; // One of NonRelaxed, Relaxed or Oracle
};

using LaneInputCondition = bool ( * )( const LaneArgs& args );

// Differences on tiers in scope are reported as known failures and don't fail the run, scoped by tier kind and feature set range
struct KnownFailure
{
    std::string_view reason;
    Tiers tiers;
    FastSIMD::FeatureSet minFeatureSet;
    FastSIMD::FeatureSet maxFeatureSet;
    bool zeroSignOnly = false;                   // Only results differing in the sign of zero are known, any other difference still fails
    LaneInputCondition inputCondition = nullptr; // Only lanes with matching inputs are known, null for every lane

    constexpr explicit KnownFailure( std::string_view knownReason, Tiers knownTiers = Tiers::All,
                                     FastSIMD::FeatureSet minKnown = FastSIMD::FeatureSet::Invalid,
                                     FastSIMD::FeatureSet maxKnown = FastSIMD::FeatureSet::Max ) :
        reason( knownReason ), tiers( knownTiers ), minFeatureSet( minKnown ), maxFeatureSet( maxKnown ) {}

    // Only results that differ in the sign of zero are known
    constexpr KnownFailure ZeroSignOnly() const
    {
        KnownFailure known = *this;
        known.zeroSignOnly = true;
        return known;
    }

    // Only lanes whose inputs match the condition are known, other inputs are still checked
    constexpr KnownFailure ForInputs( LaneInputCondition condition ) const
    {
        KnownFailure known = *this;
        known.inputCondition = condition;
        return known;
    }

    bool Covers( const TestTier& tier ) const
    {
        return ( tiers & tier.kind ) && tier.featureSet >= minFeatureSet && tier.featureSet <= maxFeatureSet;
    }

    // Any difference on a covered tier is known, so comparing more of its calls can't fail
    bool CoversAnyDifference() const
    {
        return !zeroSignOnly && !inputCondition;
    }

    bool Matches( const TestTier& tier, bool zeroSignDifference, const LaneArgs& args ) const
    {
        return Covers( tier ) && ( !zeroSignOnly || zeroSignDifference ) && ( !inputCondition || inputCondition( args ) );
    }
};

// Allowed error for relaxed results
struct Tolerance
{
    double ulps = 0;
    double absolute = 0; // For results near 0

    bool operator==( const Tolerance& other ) const { return ulps == other.ulps && absolute == other.absolute; }
    bool operator!=( const Tolerance& other ) const { return !( *this == other ); }
};

// A test as registered by one feature set, plain or relaxed
// Only the SCALAR non relaxed registration's settings (inputs, tolerances, known failures, oracle) are used, every tier must register the same ones
struct TestData
{
    enum class ReturnType
    {
        boolean, f32, f64, i32, i64 // i64 holds 64bit mask lanes
    };

    FastSIMD::FeatureSet featureSet;
    bool relaxed;
    ReturnType returnType;
    size_t lanes = 0; // Elements per call, the widest register in the test sets it
    size_t argCount = 0;
    TestArgType argTypes[kMaxTestArgs] = {};
    InputSpec inputs[kMaxTestArgs];
    Tolerance relaxedTolerance;
    std::optional<Tolerance> scalarRelaxedTolerance; // Relaxed SCALAR, when it differs from the relaxed tolerance
    bool bitExact = false;              // Float results must match bitwise on every tier, including NaN payloads
    std::vector<KnownFailure> knownFailures;
    std::function<TestFunction> testFunc;
    std::function<TestFunction> oracleFunc; // Optional plain C++ version of the test, the SCALAR reference must match it exactly
    std::function<InputsFunction> inputsFunc;
    std::function<LaneArgsFunction> laneArgsFunc;

    template<typename... SPECs>
    TestData& Inputs( SPECs... specs )
    {
        static_assert( sizeof...( SPECs ) <= kMaxTestArgs );
        size_t idx = 0;
        ( ( inputs[idx++] = specs ), ... );
        return *this;
    }

    TestData& RelaxedAccuracy( double ulps, double absolute = 0 )
    {
        relaxedTolerance = { ulps, absolute };
        return *this;
    }

    // Relaxed SCALAR uses std:: math for some functions, so this is the accuracy of the FS implementation itself
    // Relaxed SIMD tolerances cover estimate instructions, which would hide FS accuracy regressions
    TestData& ScalarRelaxedAccuracy( double ulps, double absolute = 0 )
    {
        scalarRelaxedTolerance = Tolerance{ ulps, absolute };
        return *this;
    }

    Tolerance RelaxedTolerance( FastSIMD::FeatureSet tierFeatureSet ) const
    {
        return tierFeatureSet == FastSIMD::FeatureSet::SCALAR && scalarRelaxedTolerance ? *scalarRelaxedTolerance : relaxedTolerance;
    }

    TestData& BitExact()
    {
        bitExact = true;
        return *this;
    }

    // Differences these cover are reported as known failures and don't fail the run
    template<typename... KNOWNs>
    TestData& Known( const KNOWNs&... known )
    {
        static_assert( ( std::is_same_v<KNOWNs, KnownFailure> && ... ) );
        ( knownFailures.push_back( known ), ... );
        return *this;
    }
};

using TestCollection = std::vector<std::pair<std::string_view, TestData>>;

template<size_t RegisterBytes, bool Relaxed>
class TestFastSIMD
{
public:
    virtual ~TestFastSIMD() = default;

    virtual TestCollection RegisterTests() = 0;
};
