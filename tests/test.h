#pragma once
#include <FastSIMD/DispatchClass.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string_view>
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

// Input values for each test argument, indexed by element
struct TestInputs
{
    const int32_t* ints[kMaxTestArgs];
    const float* floats[kMaxTestArgs];
    const double* doubles[kMaxTestArgs];
    const int32_t* masks[kMaxTestArgs];      // 0 or 1 per lane
    const float* maskFloats[kMaxTestArgs];   // Same lanes as masks, 0.0f or 1.0f
    const double* maskDoubles[kMaxTestArgs]; // Same lanes as masks, 0.0 or 1.0
};

enum class TestArgType : uint8_t
{
    Int, Float, Double, Mask
};

// How input values are generated for a test argument
// Every test first runs all combinations of the special values for its inputs, then random values
struct InputSpec
{
    enum class Kind
    {
        Any,       // Values of every magnitude
        Range,     // Values within [lo, hi], half uniform, half log scaled
        Magnitude, // Values with magnitude within [lo, hi] log scaled, either sign, lo > 0
    };

    Kind kind = Kind::Any;
    double lo = 0;
    double hi = 0;
    bool inf = false; // Any only, include +-inf
    bool nan = false; // Any only, include NaNs
    int significandBits = 0; // Floats are truncated to this many significand bits, 0 for all of them
    std::vector<double> specials; // Extra special values, only used when inside the spec's domain

    static InputSpec Any() { return {}; }
    static InputSpec AnyInf() { InputSpec spec; spec.inf = true; return spec; }
    static InputSpec AnyNonFinite() { InputSpec spec; spec.inf = spec.nan = true; return spec; }
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
};

using TestFunction = size_t ( void* outResult, size_t inIdx, const TestInputs& inputs );
using InputsFunction = std::string ( size_t inIdx, const TestInputs& inputs );

// Identifies what a result is checked against: a tier against the SCALAR reference, or the reference against the oracle
struct TestTier
{
    FastSIMD::FeatureSet featureSet;
    bool relaxed;
    bool oracle;
};

struct TestData
{
    enum class ReturnType
    {
        boolean, f32, f64, i32, i64 // i64 holds 64bit mask lanes
    };

    enum class Tiers
    {
        All,           // Every tier compared with the reference, not the oracle
        Relaxed,
        NonRelaxed,
        ScalarRelaxed, // Relaxed scalar uses std:: maths functions, so differs where FastSIMD is less accurate
        Oracle,        // The SCALAR reference against the test's oracle
    };

    // A lane's input values, arguments of other types are 0
    struct LaneArgs
    {
        float floats[kMaxTestArgs];
        double doubles[kMaxTestArgs];
    };

    using LaneInputCondition = bool ( * )( const LaneArgs& args );

    struct KnownFailureInfo
    {
        const char* reason;
        bool zeroSignOnly;                 // Only results differing in the sign of zero are known, any other difference still fails
        LaneInputCondition inputCondition; // Only lanes with matching inputs are known, null for every lane
        Tiers tiers;
        FastSIMD::FeatureSet minFeatureSet;
        FastSIMD::FeatureSet maxFeatureSet;

        bool Covers( const TestTier& tier ) const
        {
            bool tiersMatch = false;

            switch( tiers )
            {
            case Tiers::All:
                tiersMatch = !tier.oracle;
                break;
            case Tiers::Relaxed:
                tiersMatch = !tier.oracle && tier.relaxed;
                break;
            case Tiers::NonRelaxed:
                tiersMatch = !tier.oracle && !tier.relaxed;
                break;
            case Tiers::ScalarRelaxed:
                tiersMatch = !tier.oracle && tier.relaxed && tier.featureSet == FastSIMD::FeatureSet::SCALAR;
                break;
            case Tiers::Oracle:
                return tier.oracle;
            }

            return tiersMatch && tier.featureSet >= minFeatureSet && tier.featureSet <= maxFeatureSet;
        }

        // Any difference on a covered tier is known, so comparing more of its calls can't fail
        bool CoversAnyDifference() const
        {
            return !zeroSignOnly && !inputCondition;
        }
    };

    FastSIMD::FeatureSet featureSet;
    bool relaxed;
    ReturnType returnType;
    size_t lanes = 0; // Elements per call, the widest register in the test sets it
    size_t argCount = 0;
    TestArgType argTypes[kMaxTestArgs] = {};
    InputSpec inputs[kMaxTestArgs];
    double relaxedAccuracy = 0;          // Allowed error in ULPs for relaxed feature sets
    double relaxedAbsAccuracy = 0;       // Allowed absolute error for relaxed feature sets, for results near 0
    double scalarRelaxedAccuracy = -1;   // As above for relaxed SCALAR, negative uses the relaxed accuracy
    double scalarRelaxedAbsAccuracy = 0;
    bool bitExact = false;              // Float results must match bitwise on every tier, including NaN payloads
    std::vector<KnownFailureInfo> knownFailures;
    std::function<TestFunction> testFunc;
    std::function<TestFunction> oracleFunc; // Optional plain C++ version of the test, the SCALAR reference must match it exactly
    std::function<InputsFunction> inputsFunc;

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
        relaxedAccuracy = ulps;
        relaxedAbsAccuracy = absolute;
        return *this;
    }

    // Relaxed SCALAR uses std:: math for some functions, so this is the accuracy of the FS implementation itself
    // Relaxed SIMD tolerances cover estimate instructions, which would hide FS accuracy regressions
    TestData& ScalarRelaxedAccuracy( double ulps, double absolute = 0 )
    {
        scalarRelaxedAccuracy = ulps;
        scalarRelaxedAbsAccuracy = absolute;
        return *this;
    }

    // Allowed { ULPs, absolute } error when this tier is relaxed
    std::pair<double, double> RelaxedTolerance() const
    {
        if( featureSet == FastSIMD::FeatureSet::SCALAR && scalarRelaxedAccuracy >= 0 )
        {
            return { scalarRelaxedAccuracy, scalarRelaxedAbsAccuracy };
        }
        return { relaxedAccuracy, relaxedAbsAccuracy };
    }

    TestData& BitExact()
    {
        bitExact = true;
        return *this;
    }

    // Differences on tiers in scope are reported as known failures and don't fail the run
    // A reason is required on every tier's TestData, scope it with tiers and the feature set range
    TestData& KnownFailure( const char* reason, Tiers tiers = Tiers::All,
                            FastSIMD::FeatureSet minFeatureSet = FastSIMD::FeatureSet::Invalid,
                            FastSIMD::FeatureSet maxFeatureSet = FastSIMD::FeatureSet::Max )
    {
        knownFailures.push_back( { reason, false, nullptr, tiers, minFeatureSet, maxFeatureSet } );
        return *this;
    }

    // As KnownFailure, but only results that differ in the sign of zero are known
    TestData& KnownZeroSign( const char* reason, Tiers tiers = Tiers::All,
                             FastSIMD::FeatureSet minFeatureSet = FastSIMD::FeatureSet::Invalid,
                             FastSIMD::FeatureSet maxFeatureSet = FastSIMD::FeatureSet::Max )
    {
        knownFailures.push_back( { reason, true, nullptr, tiers, minFeatureSet, maxFeatureSet } );
        return *this;
    }

    // As KnownFailure, but only lanes whose inputs match the condition are known, other inputs are still checked
    TestData& KnownFailureIf( const char* reason, LaneInputCondition condition, Tiers tiers = Tiers::All,
                              FastSIMD::FeatureSet minFeatureSet = FastSIMD::FeatureSet::Invalid,
                              FastSIMD::FeatureSet maxFeatureSet = FastSIMD::FeatureSet::Max )
    {
        knownFailures.push_back( { reason, false, condition, tiers, minFeatureSet, maxFeatureSet } );
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
