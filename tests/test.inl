#pragma once
#include "test.h"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <tuple>

template<typename... Ts>
struct pack {};

// helper functions to retrieve arguments for member function pointers

template<class Result, class Type, class... Args>
pack<Args...> lambda_pack_helper( Result ( Type::* )( Args... ) );

template<class Result, class Type, class... Args>
pack<Args...> lambda_pack_helper( Result ( Type::* )( Args... ) const );

// helper functions: if called with an instance of the first parameter type of memorize, the return type is the one used as the second one

// overload for function
template<class Result, class... Args>
pack<Args...> pack_helper( Result( Args... ) );

// overload for function pointer
template<class Result, class... Args>
pack<Args...> pack_helper( Result ( * )( Args... ) );

// sfinae will prevent this overload from being considered for anything not providing an operator()
// for those types we use the return type of helper function lambda_pack_helper
template<class T>
decltype( lambda_pack_helper( &T::operator() ) ) pack_helper( T );


template<typename F>
struct GetReturn;

template<>
struct GetReturn<int32_t>
{
    static constexpr auto Type = TestData::ReturnType::i32;
};

template<>
struct GetReturn<float>
{
    static constexpr auto Type = TestData::ReturnType::f32;
};

template<>
struct GetReturn<double>
{
    static constexpr auto Type = TestData::ReturnType::f64;
};

template<>
struct GetReturn<bool>
{
    static constexpr auto Type = TestData::ReturnType::boolean;
};

// Mask results are stored as integer lanes of the mask's width so every bit of each lane is compared
template<size_t N, bool B>
struct GetReturn<FS::Mask<N, B>>
{
    static_assert( N == 32 || N == 64, "Mask results are stored as 32 or 64bit lanes" );
    static constexpr auto Type = N == 32 ? TestData::ReturnType::i32 : TestData::ReturnType::i64;
};

template<typename T, size_t N, FastSIMD::FeatureSet S>
struct GetReturn<FS::Register<T, N, S>>
{
    static constexpr auto Type = GetReturn<T>::Type;
};

// Elements per call of a test argument or result, scalars are 1
template<typename T>
static constexpr size_t LaneCount = 1;

template<typename T, size_t N, FastSIMD::FeatureSet S>
static constexpr size_t LaneCount<FS::Register<T, N, S>> = N;


template<FastSIMD::FeatureSet SIMD, size_t RegisterBytes, bool Relaxed>
class FastSIMD::DispatchClass<TestFastSIMD<RegisterBytes, Relaxed>, SIMD> : public TestFastSIMD<RegisterBytes, Relaxed>
{
    template<typename T>
    using TestReg = FS::Register<T, RegisterBytes / sizeof( T )>;


    template<typename T>
    static std::string AsString( T arg )
    {
        std::stringstream ss;
        ss << std::setprecision( 17 ) << std::boolalpha << arg;
        return ss.str();
    }

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    static std::string AsString( const FS::Register<T, N, S>& arg )
    {
        T scalarValues[N];
        FS::Store( scalarValues, arg );

        std::ostringstream sstr;
        sstr << "[" << AsString( scalarValues[0] );

        for( size_t idx = 1; idx < N; idx++ )
        {
            sstr << ", " << AsString( scalarValues[idx] );
        }

        sstr << "]";
        return sstr.str();
    }

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    static std::string AsString( const FS::Register<FS::Mask<MN, MB>, N, S>& arg )
    {
        FS::BitStorage<N> bitMask = FS::BitMask( arg );

        std::ostringstream sstr;
        sstr << "[" << AsString( static_cast<bool>( bitMask & 1 ) );

        for( size_t idx = 1; idx < N; idx++ )
        {
            bitMask >>= 1;

            sstr << ", " << AsString( static_cast<bool>( bitMask & 1 ) );
        }

        sstr << "]";
        return sstr.str();
    }


    template<typename T>
    static size_t StoreOutput( void* outResult, T result )
    {
        *reinterpret_cast<T*>( outResult ) = result;
        return 1;
    }

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    static size_t StoreOutput( void* outResult, FS::Register<T, N, S> result )
    {
        static_assert( N * sizeof( T ) <= kMaxTestResultBytes, "Test result is larger than the result buffer" );
        FS::Store( reinterpret_cast<T*>( outResult ), result );
        return N;
    }

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    static size_t StoreOutput( void* outResult, FS::Register<FS::Mask<MN, MB>, N, S> result )
    {
        static_assert( N * MN / 8 <= kMaxTestResultBytes, "Test result is larger than the result buffer" );

        // Store all bits of each lane, BitMask only shows the high bit
        if constexpr( MN == 32 )
        {
            FS::Store( reinterpret_cast<int32_t*>( outResult ), FS::Masked( result, FS::Register<int32_t, N, S>( -1 ) ) );
        }
        else
        {
            // No 64bit int registers, a double with every bit set keeps all the lane's bits through Masked
            static_assert( MN == 64 && MB, "Mask results are stored as 32 or 64bit lanes" );
            FS::Store( reinterpret_cast<double*>( outResult ), FS::Masked( result, FS::Register<double, N, S>( FromBits<double>( ~uint64_t( 0 ) ) ) ) );
        }
        return N;
    }


    template<typename T>
    static constexpr TestElement ElementOf()
    {
        static_assert( std::is_same_v<T, int32_t> || std::is_same_v<T, float> || std::is_same_v<T, double>, "No test inputs for type" );
        return std::is_same_v<T, float> ? TestElement::Float : std::is_same_v<T, double> ? TestElement::Double : TestElement::Int32;
    }

    // A mask argument's 0/1 lanes are stored as, and compared != 0 in: double for 64bit masks, float for float masks, int32 for int masks
    template<size_t MN, bool MB>
    using MaskElement = std::conditional_t<MN == 64, double, std::conditional_t<MB, float, int32_t>>;

    template<typename T>
    struct ArgType
    {
        static constexpr TestArgType Value = { ElementOf<T>(), false };
    };

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct ArgType<FS::Register<T, N, S>> : ArgType<T> {};

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    struct ArgType<FS::Register<FS::Mask<MN, MB>, N, S>>
    {
        static_assert( MN == 32 || ( MN == 64 && MB ), "Mask inputs are generated as 32bit lanes, or 64bit from doubles" );

        static constexpr TestArgType Value = { ElementOf<MaskElement<MN, MB>>(), true };
    };

    template<typename T>
    struct GenArg
    {
        static T Load( size_t inIdx, size_t argIdx, const TestInputs& inputs )
        {
            return inputs.Get<T>( argIdx )[inIdx];
        }
    };

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct GenArg<FS::Register<T, N, S>>
    {
        static FS::Register<T, N, S> Load( size_t inIdx, size_t argIdx, const TestInputs& inputs )
        {
            return FS::Load<N>( inputs.Get<T>( argIdx ) + inIdx );
        }
    };

    // Each mask lane is generated independently, then compared != 0 in its stored type to make the mask
    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    struct GenArg<FS::Register<FS::Mask<MN, MB>, N, S>>
    {
        static FS::Register<FS::Mask<MN, MB>, N, S> Load( size_t inIdx, size_t argIdx, const TestInputs& inputs )
        {
            using Element = MaskElement<MN, MB>;
            return FS::Load<N>( inputs.Get<Element>( argIdx ) + inIdx ) != FS::Register<Element, N, S>( Element( 0 ) );
        }
    };

    // Argument values for one lane, as plain C++ types for the oracle
    template<typename T>
    struct OracleArg
    {
        // Scalar arguments are the same for every lane
        static T Get( const TestInputs& inputs, size_t argIdx, size_t inIdx, size_t )
        {
            return inputs.Get<T>( argIdx )[inIdx];
        }
    };

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct OracleArg<FS::Register<T, N, S>>
    {
        static T Get( const TestInputs& inputs, size_t argIdx, size_t inIdx, size_t lane )
        {
            return inputs.Get<T>( argIdx )[inIdx + lane];
        }
    };

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    struct OracleArg<FS::Register<FS::Mask<MN, MB>, N, S>>
    {
        static bool Get( const TestInputs& inputs, size_t argIdx, size_t inIdx, size_t lane )
        {
            return inputs.Get<MaskElement<MN, MB>>( argIdx )[inIdx + lane] != 0;
        }
    };

    // Stores an oracle lane in the same format as StoreOutput, float/double lanes can be given as their bits in an unsigned int
    // Any unsigned type of the right size, uint64_t is unsigned long on LP64 so 64bit literals have a different type
    template<typename T, typename V>
    static void StoreOracleLane( void* outResult, size_t lane, V value )
    {
        if constexpr( std::is_floating_point_v<T> && std::is_integral_v<V> && std::is_unsigned_v<V> && sizeof( V ) == sizeof( T ) )
        {
            std::memcpy( reinterpret_cast<T*>( outResult ) + lane, &value, sizeof( T ) );
        }
        else
        {
            static_assert( std::is_same_v<T, V>, "Oracle must return the test's element type" );
            reinterpret_cast<T*>( outResult )[lane] = value;
        }
    }

    template<typename R>
    struct OracleOutput
    {
        static constexpr size_t Lanes = 1;

        template<typename V>
        static void Store( void* outResult, size_t lane, V value ) { StoreOracleLane<R>( outResult, lane, value ); }
    };

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct OracleOutput<FS::Register<T, N, S>>
    {
        static constexpr size_t Lanes = N;

        template<typename V>
        static void Store( void* outResult, size_t lane, V value ) { StoreOracleLane<T>( outResult, lane, value ); }
    };

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    struct OracleOutput<FS::Register<FS::Mask<MN, MB>, N, S>>
    {
        static constexpr size_t Lanes = N;

        static void Store( void* outResult, size_t lane, bool value )
        {
            if constexpr( MN == 32 )
            {
                reinterpret_cast<int32_t*>( outResult )[lane] = value ? -1 : 0;
            }
            else
            {
                reinterpret_cast<int64_t*>( outResult )[lane] = value ? -1 : 0;
            }
        }
    };


    template <class FUNC, class = decltype(pack_helper(std::declval<FUNC>()))>
    struct TestFunctionFactory;

    template <class FUNC, class... ARGs>
    struct TestFunctionFactory<FUNC, pack<ARGs...>>
    {
        using ReturnType = std::invoke_result_t<FUNC, ARGs...>;
        static constexpr size_t ArgCount = sizeof...( ARGs );
        static constexpr size_t Lanes = std::max( { LaneCount<ReturnType>, LaneCount<std::decay_t<ARGs>>... } );

        static_assert( Lanes <= kMaxTestLanes, "Too many lanes per call" );

        static_assert( ArgCount <= kMaxTestArgs, "Too many test arguments" );

        static void ArgTypes( TestArgType* argTypes )
        {
            [[maybe_unused]] size_t argIdx = 0;
            ( ( argTypes[argIdx++] = ArgType<std::decay_t<ARGs>>::Value ), ... );
        }

        static std::function<TestFunction> Create( FUNC func )
        {
            return [func]( void* outResult, size_t inIdx, const TestInputs& inputs )
            {
                size_t argIdx = 0;
                auto tuple = std::tuple{ GenArg<std::decay_t<ARGs>>::Load( inIdx, argIdx++, inputs )... };

                return StoreOutput( outResult, std::apply( func, tuple ) );
            };
        }

        // The oracle is called once per output lane with that lane's argument values
        template<typename ORACLE>
        static std::function<TestFunction> CreateOracle( ORACLE oracle )
        {
            return [oracle]( void* outResult, size_t inIdx, const TestInputs& inputs ) -> size_t
            {
                using Output = OracleOutput<ReturnType>;

                for( size_t lane = 0; lane < Output::Lanes; lane++ )
                {
                    [[maybe_unused]] size_t argIdx = 0;
                    auto tuple = std::tuple{ OracleArg<std::decay_t<ARGs>>::Get( inputs, argIdx++, inIdx, lane )... };

                    Output::Store( outResult, lane, std::apply( oracle, tuple ) );
                }
                return Output::Lanes;
            };
        }

        // A lane's argument values as the oracle sees them, for known failure conditions
        static LaneArgs GetLaneArgs( [[maybe_unused]] size_t inIdx, [[maybe_unused]] size_t lane, [[maybe_unused]] const TestInputs& inputs )
        {
            LaneArgs args = {};
            [[maybe_unused]] size_t argIdx = 0;
            ( ( args.Set( argIdx, OracleArg<std::decay_t<ARGs>>::Get( inputs, argIdx, inIdx, lane ) ), argIdx++ ), ... );
            return args;
        }

        static std::function<InputsFunction> GetInputs()
        {
            return []( size_t inIdx, const TestInputs& inputs ) -> std::string
            {
                if( !sizeof...( ARGs ) )
                {
                    return "No Inputs";
                }

                size_t argIdx = 0;
                std::initializer_list<std::string> tuple = { AsString( GenArg<std::decay_t<ARGs>>::Load( inIdx, argIdx++, inputs ) )... };

                std::ostringstream inputsString;
                inputsString << "\n";

                std::copy( tuple.begin(), tuple.end() - 1, std::ostream_iterator<std::string>( inputsString, "\n" ) );

                inputsString << *(tuple.end() - 1);

                return inputsString.str();
            };
        }
    };


    template<typename FUNC>
    TestData& RegisterTest( TestCollection& testMap, const char* name, FUNC func )
    {
        TestData data;
        data.featureSet = SIMD;
        data.relaxed = FastSIMD::IsRelaxed();
        data.returnType = GetReturn<typename TestFunctionFactory<FUNC>::ReturnType>::Type;
        data.lanes = TestFunctionFactory<FUNC>::Lanes;
        data.argCount = TestFunctionFactory<FUNC>::ArgCount;
        TestFunctionFactory<FUNC>::ArgTypes( data.argTypes );

        data.testFunc = TestFunctionFactory<FUNC>::Create( func );
        data.inputsFunc = TestFunctionFactory<FUNC>::GetInputs();
        data.laneArgsFunc = TestFunctionFactory<FUNC>::GetLaneArgs;

        return testMap.emplace_back( name, data ).second;
    }

    // oracle is a plain C++ version of the test, called per lane with plain values (masks as bool)
    template<typename FUNC, typename ORACLE>
    TestData& RegisterTest( TestCollection& testMap, const char* name, FUNC func, ORACLE oracle )
    {
        TestData& data = RegisterTest( testMap, name, func );
        data.oracleFunc = TestFunctionFactory<FUNC>::CreateOracle( oracle );
        return data;
    }

    // int32 arithmetic wraps on every tier, done in uint32 to avoid signed overflow
    static int32_t Wrap( uint32_t v ) { return static_cast<int32_t>( v ); }
    static uint32_t U( int32_t v ) { return static_cast<uint32_t>( v ); }

    // x + 0.0f as the masked add/decrement implementations do: -0 becomes +0, everything else is unchanged
    static float PlusZero( float a ) { return a == 0.0f ? 0.0f : a; }

    using TestRegi32 = TestReg<int32_t>;
    using TestRegf32 = TestReg<float>;
    using TestRegm32 = typename TestRegf32::MaskType;
    using TestRegm32i = typename TestRegi32::MaskType;
    using In = InputSpec;

    static constexpr double denormMin = std::numeric_limits<float>::denorm_min();
    static constexpr double floatMax = std::numeric_limits<float>::max();
    static constexpr double pi = 3.14159265358979323846;

    // Lane input conditions for known failures
    static bool DenormalArg0( const LaneArgs& args ) { return std::fpclassify( args.floats[0] ) == FP_SUBNORMAL; }
    static bool DenormalArg1( const LaneArgs& args ) { return std::fpclassify( args.floats[1] ) == FP_SUBNORMAL; }
    static bool QuotientOverflows( const LaneArgs& args ) { return std::isfinite( args.floats[0] ) && args.floats[1] != 0 && std::isinf( args.floats[0] / args.floats[1] ); }
    static bool X86ReciprocalInput( const LaneArgs& args ) { return std::fpclassify( args.floats[0] ) == FP_SUBNORMAL || std::abs( args.floats[0] ) >= 0x1.ffcp125f; }
    static bool ArmEstimateInput( const LaneArgs& args ) { return args.floats[0] == 0 || std::isinf( args.floats[0] ) || std::fpclassify( args.floats[0] ) == FP_SUBNORMAL; }

    // Known library behaviour, each scoped to the tiers that show it
    static constexpr KnownFailure sse2RoundZeroSign = KnownFailure(
        "Below SSE4.1 rounding converts through int32 (cvtps2dq/cvttps2dq), which has no -0, so negative results that round to 0 are +0",
        Tiers::NonRelaxed, FeatureSet::SSE, FeatureSet::SSSE3 ).ZeroSignOnly();
    static constexpr KnownFailure sinReduceZeroSign = KnownFailure(
        "Sin_Reduce uses Trunc, see f32 trunc: Sin( -0 ) is -0 below SSE4.1 and +0 elsewhere",
        Tiers::NonRelaxed, FeatureSet::SSE, FeatureSet::SSSE3 ).ZeroSignOnly();
    static constexpr KnownFailure neonDivide = KnownFailure(
        "NEON (ARMv7) has no divide, division is a reciprocal estimate plus Newton-Raphson steps which isn't correctly rounded",
        Tiers::NonRelaxed, FeatureSet::NEON, FeatureSet::NEON );
    // vrecpe overflows to inf for |x| < 2^-128, then vrecps( x, inf ) is -inf, so the Newton-Raphson steps give inf of the wrong sign, or NaN for 0 / x
    static constexpr KnownFailure neonDivideDenormal = KnownFailure(
        "NEON (ARMv7) divide by a denormal: the reciprocal estimate overflows to inf and the Newton-Raphson steps flip its sign, giving -inf for +inf and NaN for 0",
        Tiers::Relaxed, FeatureSet::NEON, FeatureSet::NEON ).ForInputs( DenormalArg1 );
    // The refined reciprocal can be 1 ULP low, e.g. 1 / 0.99999994 becomes 1, so FLT_MAX / 0.99999994 is FLT_MAX instead of inf
    static constexpr KnownFailure neonDivideOverflow = KnownFailure(
        "NEON (ARMv7) divide near overflow: the Newton-Raphson reciprocal can be 1 ULP low, so quotients that overflow to +-inf can give +-FLT_MAX",
        Tiers::Relaxed, FeatureSet::NEON, FeatureSet::NEON ).ForInputs( QuotientOverflows );
    // Generic masked add is a + Masked( m, b ), so masked off lanes add +0. AVX512 512bit uses a native masked add that leaves them unchanged
    static constexpr KnownFailure avx512MaskedAddZeroSign = KnownFailure(
        "AVX512 512bit masked add leaves masked off lanes unchanged, the generic a + Masked( m, b ) turns -0 into +0",
        RegisterBytes >= 64 ? Tiers::NonRelaxed : Tiers::None, FeatureSet::AVX512, FeatureSet::AVX512 ).ZeroSignOnly();
    // Relaxed x86 uses rcpps/rsqrtps, AVX512 512bit registers use rcp14/rsqrt14 which handle denormals
    static constexpr FastSIMD::FeatureSet x86EstimateMax = RegisterBytes >= 64 ? FeatureSet::AVX2 : FeatureSet::AVX512;
    static constexpr KnownFailure x86EstimateDenormals = KnownFailure(
        "Relaxed rcpps/rsqrtps estimates treat denormal inputs as 0 and return inf",
        Tiers::Relaxed, FeatureSet::SSE, x86EstimateMax ).ForInputs( DenormalArg0 );
    // The Intel SDM allows rcpps to flush results just above FLT_MIN to 0, for |x| just below 2^126
    static constexpr KnownFailure x86ReciprocalEstimate = KnownFailure(
        "Relaxed rcpps treats denormal inputs as 0 and returns inf, and may flush results near FLT_MIN to 0",
        Tiers::Relaxed, FeatureSet::SSE, x86EstimateMax ).ForInputs( X86ReciprocalInput );
    // Relaxed ARM refines vrsqrte/vrecpe with one Newton-Raphson step, which breaks where the estimate is 0 or inf
    static constexpr KnownFailure armEstimateSpecials = KnownFailure(
        "Relaxed ARM estimate + Newton-Raphson step: InvSqrt( +-0 ) and InvSqrt( inf ) are NaN (0 * inf), Reciprocal of a denormal is inf of the wrong sign",
        Tiers::Relaxed, FeatureSet::NEON, FeatureSet::AARCH64 ).ForInputs( ArmEstimateInput );
#if defined( _MSC_VER ) && !defined( __clang__ ) && defined( _DEBUG )
    // The scalar reference is wrong, so every tier and the oracle comparison see it
    static constexpr Tiers msvcDebugAbsTiers = Tiers::All | Tiers::Oracle;
#else
    static constexpr Tiers msvcDebugAbsTiers = Tiers::None;
#endif
    static constexpr KnownFailure msvcDebugAbs = KnownFailure(
        "Unoptimised MSVC calls the CRT fabsf for std::abs in the scalar FS::Abs, which quiets signalling NaNs", msvcDebugAbsTiers );

    TestCollection RegisterTests() override
    {
        TestCollection tests;
        RegisterMaskTests( tests );
        RegisterIntTests( tests );
        RegisterFloatTests( tests );
        RegisterFloatMathTests( tests );
        RegisterConvertTests( tests );
        return tests;
    }

    void RegisterMaskTests( TestCollection& tests )
    {
        RegisterTest( tests, "m32 bit mask", []( TestRegm32 a ) { return a; }, []( bool a ) { return a; } );
        RegisterTest( tests, "m32 any mask", []( TestRegm32 a ) { return FS::AnyMask( a ); } );
        RegisterTest( tests, "m32 bit mask to int", []( TestRegm32 a ) { return static_cast<int32_t>( FS::BitMask( a ) ); } );

        RegisterTest( tests, "m32 bit and operator", std::bit_and<TestRegm32>(), []( bool a, bool b ) { return a && b; } );
        RegisterTest( tests, "m32 bit or operator", std::bit_or<TestRegm32>(), []( bool a, bool b ) { return a || b; } );
        RegisterTest( tests, "m32 bit xor operator", std::bit_xor<TestRegm32>(), []( bool a, bool b ) { return a != b; } );
        RegisterTest( tests, "m32 bit not operator", std::bit_not<TestRegm32>(), []( bool a ) { return !a; } );
        RegisterTest( tests, "m32 bit and not", []( TestRegm32 a, TestRegm32 b ) { return FS::BitwiseAndNot( a, b ); }, []( bool a, bool b ) { return a && !b; } );

        RegisterTest( tests, "m32i bit mask", []( TestRegm32i a ) { return a; }, []( bool a ) { return a; } );
        RegisterTest( tests, "m32i any mask", []( TestRegm32i a ) { return FS::AnyMask( a ); } );
        RegisterTest( tests, "m32i bit mask to int", []( TestRegm32i a ) { return static_cast<int32_t>( FS::BitMask( a ) ); } );

        RegisterTest( tests, "m32i bit and operator", std::bit_and<TestRegm32i>(), []( bool a, bool b ) { return a && b; } );
        RegisterTest( tests, "m32i bit or operator", std::bit_or<TestRegm32i>(), []( bool a, bool b ) { return a || b; } );
        RegisterTest( tests, "m32i bit xor operator", std::bit_xor<TestRegm32i>(), []( bool a, bool b ) { return a != b; } );
        RegisterTest( tests, "m32i bit not operator", std::bit_not<TestRegm32i>(), []( bool a ) { return !a; } );
        RegisterTest( tests, "m32i bit and not", []( TestRegm32i a, TestRegm32i b ) { return FS::BitwiseAndNot( a, b ); }, []( bool a, bool b ) { return a && !b; } );
    }

    void RegisterIntTests( TestCollection& tests )
    {
        RegisterTest( tests, "i32 load store", []( TestRegi32 a ) { return a; }, []( int32_t a ) { return a; } );
        RegisterTest( tests, "i32 load scalar", []( int32_t a ) { return TestRegi32( a ); }, []( int32_t a ) { return a; } );
        RegisterTest( tests, "i32 splat", []( int32_t a ) { return FS::Splat<TestRegi32::ElementCount>( a ); }, []( int32_t a ) { return a; } );
        RegisterTest( tests, "i32 extract 0", []( TestRegi32 a ) { return FS::Extract0( a ); }, []( int32_t a ) { return a; } );
        RegisterTest( tests, "i32 load incremented", []() { return FS::LoadIncremented<TestRegi32>(); } );

        RegisterTest( tests, "i32 plus operator", std::plus<TestRegi32>(), []( int32_t a, int32_t b ) { return Wrap( U( a ) + U( b ) ); } );
        RegisterTest( tests, "i32 minus operator", std::minus<TestRegi32>(), []( int32_t a, int32_t b ) { return Wrap( U( a ) - U( b ) ); } );
        RegisterTest( tests, "i32 multiply operator", std::multiplies<TestRegi32>(), []( int32_t a, int32_t b ) { return Wrap( U( a ) * U( b ) ); } );

        RegisterTest( tests, "i32 bit and operator", std::bit_and<TestRegi32>(), []( int32_t a, int32_t b ) { return a & b; } );
        RegisterTest( tests, "i32 bit or operator", std::bit_or<TestRegi32>(), []( int32_t a, int32_t b ) { return a | b; } );
        RegisterTest( tests, "i32 bit xor operator", std::bit_xor<TestRegi32>(), []( int32_t a, int32_t b ) { return a ^ b; } );
        RegisterTest( tests, "i32 bit not operator", std::bit_not<TestRegi32>(), []( int32_t a ) { return ~a; } );
        RegisterTest( tests, "i32 negate operator", std::negate<TestRegi32>(), []( int32_t a ) { return Wrap( 0u - U( a ) ); } );
        // MSVC x86 has miscompiled operators taking an over-aligned lhs by value, the copy was elided and the result written into the caller's a
        // Select( m, a OP b, a ) reads a again after each operator, the shape that showed it
        RegisterTest( tests, "i32 operators keep lhs", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) {
            return FS::Select( m, a + b, a ) ^ FS::Select( m, a - b, a ) ^ FS::Select( m, a * b, a ) ^ FS::Select( m, a & b, a ) ^
                   FS::Select( m, a | b, a ) ^ FS::Select( m, a ^ b, a ) ^ FS::Select( m, a << 3, a ) ^ FS::Select( m, a >> 3, a ) ^ a;
        }, []( bool m, int32_t a, int32_t b ) {
            int32_t results[] = { Wrap( U( a ) + U( b ) ), Wrap( U( a ) - U( b ) ), Wrap( U( a ) * U( b ) ), a & b, a | b, a ^ b, Wrap( U( a ) << 3 ), a < 0 ? ~( ~a >> 3 ) : a >> 3 };
            int32_t combined = a;
            for( int32_t result : results )
            {
                combined ^= m ? result : a;
            }
            return combined;
        } );
        RegisterTest( tests, "i32 bit and not", []( TestRegi32 a, TestRegi32 b ) { return FS::BitwiseAndNot( a, b ); }, []( int32_t a, int32_t b ) { return a & ~b; } );

        RegisterTest( tests, "i32 increment", []( TestRegi32 a ) { return FS::Increment( a ); }, []( int32_t a ) { return Wrap( U( a ) + 1u ); } );
        RegisterTest( tests, "i32 decrement", []( TestRegi32 a ) { return FS::Decrement( a ); }, []( int32_t a ) { return Wrap( U( a ) - 1u ); } );
        RegisterTest( tests, "i32 abs", []( TestRegi32 a ) { return FS::Abs( a ); }, []( int32_t a ) { return a < 0 ? Wrap( 0u - U( a ) ) : a; } );
        RegisterTest( tests, "i32 min", []( TestRegi32 a, TestRegi32 b ) { return FS::Min( a, b ); }, []( int32_t a, int32_t b ) { return std::min( a, b ); } );
        RegisterTest( tests, "i32 max", []( TestRegi32 a, TestRegi32 b ) { return FS::Max( a, b ); }, []( int32_t a, int32_t b ) { return std::max( a, b ); } );

        RegisterTest( tests, "i32 bit shift left scalar", []( TestRegi32 a, int b ) { return a << b; }, []( int32_t a, int32_t b ) { return Wrap( U( a ) << b ); } ).Inputs( In(), In::Range( 0, 31 ) );
        RegisterTest( tests, "i32 bit shift right scalar", []( TestRegi32 a, int b ) { return a >> b; }, []( int32_t a, int32_t b ) { return a < 0 ? ~( ~a >> b ) : a >> b; } ).Inputs( In(), In::Range( 0, 31 ) );
        RegisterTest( tests, "i32 bit shift right zero extend scalar", []( TestRegi32 a, int b ) { return FS::BitShiftRightZeroExtend( a, b ); }, []( int32_t a, int32_t b ) { return Wrap( U( a ) >> b ); } ).Inputs( In(), In::Range( 0, 31 ) );

        //RegisterTest( tests, "i32 bit shift left", []( TestRegi32 a, TestRegi32 b ) { return a << FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );
        //RegisterTest( tests, "i32 bit shift right", []( TestRegi32 a, TestRegi32 b ) { return a >> FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );

        RegisterTest( tests, "i32 equals operator", []( TestRegi32 a, TestRegi32 b ) { return a == b; }, []( int32_t a, int32_t b ) { return a == b; } );
        RegisterTest( tests, "i32 equals operator alt", []( TestRegi32 a ) { return a == a; }, []( int32_t ) { return true; } );
        RegisterTest( tests, "i32 not equals operator", []( TestRegi32 a, TestRegi32 b ) { return a != b; }, []( int32_t a, int32_t b ) { return a != b; } );
        RegisterTest( tests, "i32 not equals operator alt", []( TestRegi32 a ) { return a != a; }, []( int32_t ) { return false; } );
        RegisterTest( tests, "i32 less than operator", []( TestRegi32 a, TestRegi32 b ) { return a < b; }, []( int32_t a, int32_t b ) { return a < b; } );
        RegisterTest( tests, "i32 greater than operator", []( TestRegi32 a, TestRegi32 b ) { return a > b; }, []( int32_t a, int32_t b ) { return a > b; } );
        RegisterTest( tests, "i32 less equal than operator", []( TestRegi32 a, TestRegi32 b ) { return a <= b; }, []( int32_t a, int32_t b ) { return a <= b; } );
        RegisterTest( tests, "i32 greater equal than operator", []( TestRegi32 a, TestRegi32 b ) { return a >= b; }, []( int32_t a, int32_t b ) { return a >= b; } );

        RegisterTest( tests, "i32 select", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::Select( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? a : b; } );
        // i32's own mask type, as int compares return
        RegisterTest( tests, "i32 select int mask", []( TestRegm32i m, TestRegi32 a, TestRegi32 b ) { return FS::Select( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? a : b; } );
        RegisterTest( tests, "i32 masked int mask", []( TestRegm32i m, TestRegi32 a ) { return FS::Masked( m, a ); }, []( bool m, int32_t a ) { return m ? a : 0; } );
        RegisterTest( tests, "i32 masked increment int mask", []( TestRegm32i m, TestRegi32 a ) { return FS::MaskedIncrement( m, a ); }, []( bool m, int32_t a ) { return m ? Wrap( U( a ) + 1u ) : a; } );
        RegisterTest( tests, "i32 select high bit", []( TestRegf32 m, TestRegi32 a, TestRegi32 b ) { return FS::SelectHighBit( m, a, b ); },
                      []( float m, int32_t a, int32_t b ) { return ( ToBits( m ) >> 31 ) ? a : b; } );
        RegisterTest( tests, "i32 masked", []( TestRegm32 m, TestRegi32 a ) { return FS::Masked( m, a ); }, []( bool m, int32_t a ) { return m ? a : 0; } );
        RegisterTest( tests, "i32 inv masked", []( TestRegm32 m, TestRegi32 a ) { return FS::InvMasked( m, a ); }, []( bool m, int32_t a ) { return m ? 0 : a; } );
        RegisterTest( tests, "i32 masked increment", []( TestRegm32 m, TestRegi32 a ) { return FS::MaskedIncrement( m, a ); }, []( bool m, int32_t a ) { return m ? Wrap( U( a ) + 1u ) : a; } );
        RegisterTest( tests, "i32 masked decrement", []( TestRegm32 m, TestRegi32 a ) { return FS::MaskedDecrement( m, a ); }, []( bool m, int32_t a ) { return m ? Wrap( U( a ) - 1u ) : a; } );
        RegisterTest( tests, "i32 masked add", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::MaskedAdd( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? Wrap( U( a ) + U( b ) ) : a; } );
        RegisterTest( tests, "i32 masked sub", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::MaskedSub( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? Wrap( U( a ) - U( b ) ) : a; } );
        RegisterTest( tests, "i32 masked mul", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::MaskedMul( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? Wrap( U( a ) * U( b ) ) : a; } );
        RegisterTest( tests, "i32 inv masked add", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::InvMaskedAdd( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? a : Wrap( U( a ) + U( b ) ); } );
        RegisterTest( tests, "i32 inv masked sub", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::InvMaskedSub( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? a : Wrap( U( a ) - U( b ) ); } );
        RegisterTest( tests, "i32 inv masked mul", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::InvMaskedMul( m, a, b ); }, []( bool m, int32_t a, int32_t b ) { return m ? a : Wrap( U( a ) * U( b ) ); } );
    }

    void RegisterFloatTests( TestCollection& tests )
    {
        RegisterTest( tests, "f32 load store", []( TestRegf32 a ) { return a; }, []( float a ) { return ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 load scalar", []( float a ) { return TestRegf32( a ); }, []( float a ) { return ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 splat", []( float a ) { return FS::Splat<TestRegf32::ElementCount>( a ); }, []( float a ) { return ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 extract 0", []( TestRegf32 a ) { return FS::Extract0( a ); }, []( float a ) { return ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 load incremented", []() { return FS::LoadIncremented<TestRegf32>(); } );

        RegisterTest( tests, "f32 plus operator", std::plus<TestRegf32>(), []( float a, float b ) { return a + b; } );
        RegisterTest( tests, "f32 minus operator", std::minus<TestRegf32>(), []( float a, float b ) { return a - b; } );
        RegisterTest( tests, "f32 multiply operator", std::multiplies<TestRegf32>(), []( float a, float b ) { return a * b; } );
        RegisterTest( tests, "f32 divide operator", std::divides<TestRegf32>(), []( float a, float b ) { return a / b; } ).RelaxedAccuracy( 64 )
            .Known( neonDivide, neonDivideDenormal, neonDivideOverflow );

        // b = -1 keeps a * b exact, so fused and unfused results match
        RegisterTest( tests, "f32 fused multiply add", []( TestRegf32 a, TestRegf32 b ) { return FS::FMulAdd( a, TestRegf32( -1 ), b ); }, []( float a, float b ) { return -a + b; } );
        RegisterTest( tests, "f32 fused multiply sub", []( TestRegf32 a, TestRegf32 b ) { return FS::FMulSub( a, TestRegf32( -1 ), b ); }, []( float a, float b ) { return -a - b; } );
        RegisterTest( tests, "f32 fused negative multiply add", []( TestRegf32 a, TestRegf32 b ) { return FS::FNMulAdd( a, TestRegf32( -1 ), b ); }, []( float a, float b ) { return b + a; } );
        RegisterTest( tests, "f32 fused negative multiply sub", []( TestRegf32 a, TestRegf32 b ) { return FS::FNMulSub( a, TestRegf32( -1 ), b ); }, []( float a, float b ) { return -b + a; } );

        // 12 bit significands keep a * b exact, so fused and unfused results match while every argument is independent
        const In exactProduct = In::Magnitude( 1.e-15, 1.e+15 ).Bits( 12 );
        RegisterTest( tests, "f32 fused multiply add exact product", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FMulAdd( a, b, c ); },
                      []( float a, float b, float c ) { float ab = a * b; return ab + c; } ).Inputs( exactProduct, exactProduct, In::Range( -1.e+30, 1.e+30 ) );
        RegisterTest( tests, "f32 fused multiply sub exact product", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FMulSub( a, b, c ); },
                      []( float a, float b, float c ) { float ab = a * b; return ab - c; } ).Inputs( exactProduct, exactProduct, In::Range( -1.e+30, 1.e+30 ) );
        RegisterTest( tests, "f32 fused negative multiply add exact product", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FNMulAdd( a, b, c ); },
                      []( float a, float b, float c ) { float ab = a * b; return c - ab; } ).Inputs( exactProduct, exactProduct, In::Range( -1.e+30, 1.e+30 ) );
        RegisterTest( tests, "f32 fused negative multiply sub exact product", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FNMulSub( a, b, c ); },
                      []( float a, float b, float c ) { float ab = a * b; return -c - ab; } ).Inputs( exactProduct, exactProduct, In::Range( -1.e+30, 1.e+30 ) );

        RegisterTest( tests, "f32 bit and operator", std::bit_and<TestRegf32>(), []( float a, float b ) { return ToBits( a ) & ToBits( b ); } ).BitExact();
        RegisterTest( tests, "f32 bit or operator", std::bit_or<TestRegf32>(), []( float a, float b ) { return ToBits( a ) | ToBits( b ); } ).BitExact();
        RegisterTest( tests, "f32 bit xor operator", std::bit_xor<TestRegf32>(), []( float a, float b ) { return ToBits( a ) ^ ToBits( b ); } ).BitExact();
        RegisterTest( tests, "f32 bit not operator", std::bit_not<TestRegf32>(), []( float a ) { return ~ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 negate operator", std::negate<TestRegf32>(), []( float a ) { return ToBits( a ) ^ 0x80000000u; } ).BitExact();
        // See i32 operators keep lhs. Divide isn't exact on NEON, its lhs is read again by FS::Modulus, see f32 modulus exact
        RegisterTest( tests, "f32 operators keep lhs", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) {
            return FS::Select( m, a + b, a ) ^ FS::Select( m, a - b, a ) ^ FS::Select( m, a * b, a ) ^
                   FS::Select( m, a & b, a ) ^ FS::Select( m, a | b, a ) ^ FS::Select( m, a ^ b, a ) ^ a;
        }, []( bool m, float a, float b ) {
            uint32_t results[] = { ToBits( a + b ), ToBits( a - b ), ToBits( a * b ), ToBits( a ) & ToBits( b ), ToBits( a ) | ToBits( b ), ToBits( a ) ^ ToBits( b ) };
            uint32_t combined = ToBits( a );
            for( uint32_t result : results )
            {
                combined ^= m ? result : ToBits( a );
            }
            return combined;
        } ).Inputs( In(), In::Finite(), In::Finite() ).BitExact();
        RegisterTest( tests, "f32 bit and not", []( TestRegf32 a, TestRegf32 b ) { return FS::BitwiseAndNot( a, b ); }, []( float a, float b ) { return ToBits( a ) & ~ToBits( b ); } ).BitExact();

        RegisterTest( tests, "f32 equals operator", []( TestRegf32 a, TestRegf32 b ) { return a == b; }, []( float a, float b ) { return a == b; } );
        RegisterTest( tests, "f32 greater equal than operator", []( TestRegf32 a, TestRegf32 b ) { return a >= b; }, []( float a, float b ) { return a >= b; } );
        RegisterTest( tests, "f32 not equals operator", []( TestRegf32 a, TestRegf32 b ) { return a != b; }, []( float a, float b ) { return a != b; } );
        RegisterTest( tests, "f32 less than operator", []( TestRegf32 a, TestRegf32 b ) { return a < b; }, []( float a, float b ) { return a < b; } );
        RegisterTest( tests, "f32 greater than operator", []( TestRegf32 a, TestRegf32 b ) { return a > b; }, []( float a, float b ) { return a > b; } );
        RegisterTest( tests, "f32 less equal than operator", []( TestRegf32 a, TestRegf32 b ) { return a <= b; }, []( float a, float b ) { return a <= b; } );

        RegisterTest( tests, "f32 increment", []( TestRegf32 a ) { return FS::Increment( a ); }, []( float a ) { return a + 1.0f; } );
        RegisterTest( tests, "f32 decrement", []( TestRegf32 a ) { return FS::Decrement( a ); }, []( float a ) { return a - 1.0f; } );
        RegisterTest( tests, "f32 abs", []( TestRegf32 a ) { return FS::Abs( a ); }, []( float a ) { return ToBits( a ) & 0x7FFFFFFFu; } ).BitExact().Known( msvcDebugAbs );
        // Relaxed min/max may use instructions with different NaN handling, those are checked non relaxed in the full range tests
        RegisterTest( tests, "f32 min", []( TestRegf32 a, TestRegf32 b ) { return FS::Min( a, b ); }, []( float a, float b ) { return std::min( a, b ); } ).Inputs( In::NoNaN(), In::NoNaN() );
        RegisterTest( tests, "f32 max", []( TestRegf32 a, TestRegf32 b ) { return FS::Max( a, b ); }, []( float a, float b ) { return std::max( a, b ); } ).Inputs( In::NoNaN(), In::NoNaN() );

        RegisterTest( tests, "f32 select", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::Select( m, a, b ); }, []( bool m, float a, float b ) { return ToBits( m ? a : b ); } ).BitExact();
        RegisterTest( tests, "f32 select high bit", []( TestRegi32 m, TestRegf32 a, TestRegf32 b ) { return FS::SelectHighBit( m, a, b ); }, []( int32_t m, float a, float b ) { return ToBits( m < 0 ? a : b ); } ).BitExact();
        RegisterTest( tests, "f32 masked", []( TestRegm32 m, TestRegf32 a ) { return FS::Masked( m, a ); }, []( bool m, float a ) { return m ? ToBits( a ) : 0u; } ).BitExact();
        RegisterTest( tests, "f32 inv masked", []( TestRegm32 m, TestRegf32 a ) { return FS::InvMasked( m, a ); }, []( bool m, float a ) { return m ? 0u : ToBits( a ); } ).BitExact();
        RegisterTest( tests, "f32 masked increment", []( TestRegm32 m, TestRegf32 a ) { return FS::MaskedIncrement( m, a ); }, []( bool m, float a ) { return m ? a + 1.0f : a; } );
        RegisterTest( tests, "f32 masked decrement", []( TestRegm32 m, TestRegf32 a ) { return FS::MaskedDecrement( m, a ); }, []( bool m, float a ) { return m ? a - 1.0f : PlusZero( a ); } );

        RegisterTest( tests, "f32 masked add", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::MaskedAdd( m, a, b ); },
                      []( bool m, float a, float b ) { return m ? a + b : PlusZero( a ); } ).Known( avx512MaskedAddZeroSign );
        RegisterTest( tests, "f32 masked sub", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::MaskedSub( m, a, b ); }, []( bool m, float a, float b ) { return m ? a - b : a; } );
        RegisterTest( tests, "f32 masked mul", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::MaskedMul( m, a, b ); }, []( bool m, float a, float b ) { return m ? a * b : a; } );
        RegisterTest( tests, "f32 inv masked add", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::InvMaskedAdd( m, a, b ); },
                      []( bool m, float a, float b ) { return m ? PlusZero( a ) : a + b; } ).Known( avx512MaskedAddZeroSign );
        RegisterTest( tests, "f32 inv masked sub", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::InvMaskedSub( m, a, b ); }, []( bool m, float a, float b ) { return m ? a : a - b; } );
        RegisterTest( tests, "f32 inv masked mul", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::InvMaskedMul( m, a, b ); }, []( bool m, float a, float b ) { return m ? a : a * b; } );

        RegisterTest( tests, "f32 round", []( TestRegf32 a ) { return FS::Round( a ); }, []( float a ) { return std::rint( a ); } )
            .Known( sse2RoundZeroSign );
        RegisterTest( tests, "f32 ceil", []( TestRegf32 a ) { return FS::Ceil( a ); }, []( float a ) { return std::ceil( a ); } )
            .Known( sse2RoundZeroSign );
        RegisterTest( tests, "f32 floor", []( TestRegf32 a ) { return FS::Floor( a ); }, []( float a ) { return std::floor( a ); } )
            .Known( sse2RoundZeroSign );
        RegisterTest( tests, "f32 trunc", []( TestRegf32 a ) { return FS::Trunc( a ); }, []( float a ) { return std::trunc( a ); } )
            .Known( sse2RoundZeroSign );
        RegisterTest( tests, "f32 signbit", []( TestRegf32 a, TestRegf32 b ) { return FS::SignBit( a ) ^ b; }, []( float a, float b ) { return ( ToBits( a ) & 0x80000000u ) ^ ToBits( b ); } ).BitExact();
        RegisterTest( tests, "f32 modulus", []( TestRegf32 a, TestRegf32 b ) { return FS::Modulus( a, b ); } )
            .Inputs( In::Range( -100, 100 ), In::Magnitude( 0.25, 100 ) ).RelaxedAccuracy( 16384 )
            .Known( KnownFailure( "Scalar uses std::fmod, SIMD and relaxed use a - trunc( a / b ) * b which isn't exact" ) );
        // b is a power of 2 and a is never tiny, so a - trunc( a / b ) * b is exact and must match std::fmod
        RegisterTest( tests, "f32 modulus exact", []( TestRegf32 a, TestRegf32 b ) { return FS::Modulus( a, b ); }, []( float a, float b ) { return std::fmod( a, b ); } )
            .Inputs( In::Magnitude( 1.e-30, 100 ), In::Magnitude( 0.25, 64 ).Bits( 1 ) )
            .Known( KnownFailure( "a - trunc( a / b ) * b gives +0 for exact negative multiples, std::fmod gives -0", Tiers::NonRelaxed ).ZeroSignOnly() );
    }

    void RegisterFloatMathTests( TestCollection& tests )
    {
        // Range reduction boundaries for trig
        const In trigRange = In::Range( -4096, 4096 ).Specials( { pi / 4, pi / 2, pi, 3 * pi / 2, 2 * pi, 4 * pi, 100 * pi, 1000 * pi,
                                                                  -pi / 4, -pi / 2, -pi, -3 * pi / 2, -2 * pi, -4 * pi, -100 * pi, -1000 * pi } );
        const In smallTrigRange = In::Range( -2 * pi, 2 * pi ).Specials( { pi / 4, pi / 2, pi, 3 * pi / 2, 2 * pi,
                                                                           -pi / 4, -pi / 2, -pi, -3 * pi / 2, -2 * pi } );

        RegisterTest( tests, "f32 sqrt", []( TestRegf32 a ) { return FS::Sqrt( a ); }, []( float a ) { return std::sqrt( a ); } );
        // Relaxed x86 uses the rcpps/rsqrtps estimates, their 1.5 * 2^-12 relative error is up to ~6144 ULP
        RegisterTest( tests, "f32 inv sqrt", []( TestRegf32 a ) { return FS::InvSqrt( a ); }, []( float a ) { return 1.0f / std::sqrt( a ); } ).Inputs( In::Range( 1.e-16, 1.e+16 ) ).RelaxedAccuracy( 8192 )
            .Known( neonDivide );
        RegisterTest( tests, "f32 reciprocal", []( TestRegf32 a ) { return FS::Reciprocal( a ); }, []( float a ) { return 1.0f / a; } ).Inputs( In::Magnitude( 1.e-16, 1.e+16 ) ).RelaxedAccuracy( 8192 )
            .Known( neonDivide );

        // Absolute error allowance is ~1 ULP of the largest input, float inputs that large are only that precise
        RegisterTest( tests, "f32 cos", []( TestRegf32 a ) { return FS::Cos( a ); } ).Inputs( trigRange ).RelaxedAccuracy( 4, 5.e-4f );
        RegisterTest( tests, "f32 sin", []( TestRegf32 a ) { return FS::Sin( a ); } ).Inputs( trigRange ).RelaxedAccuracy( 4, 5.e-4f )
            .Known( sinReduceZeroSign );
        // Within +-2pi range reduction is exact apart from the float rounding of pi and 2pi (<= ~2.6e-7), so the polynomials are checked closely
        RegisterTest( tests, "f32 cos small range", []( TestRegf32 a ) { return FS::Cos( a ); } ).Inputs( smallTrigRange ).RelaxedAccuracy( 4, 5.e-7f );
        RegisterTest( tests, "f32 sin small range", []( TestRegf32 a ) { return FS::Sin( a ); } ).Inputs( smallTrigRange ).RelaxedAccuracy( 4, 5.e-7f )
            .Known( sinReduceZeroSign );
        // The absolute allowances above hide relative accuracy near 0
        RegisterTest( tests, "f32 sin near 0", []( TestRegf32 a ) { return FS::Sin( a ); } ).Inputs( In::Magnitude( denormMin, 0.01 ) ).RelaxedAccuracy( 4 );
        // Tan is ill conditioned near its poles, so relaxed accuracy is checked between them
        RegisterTest( tests, "f32 tan", []( TestRegf32 a ) { return FS::Tan( a ); } ).Inputs( In::Range( -1.5, 1.5 ).Specials( { pi / 4, -pi / 4 } ) ).RelaxedAccuracy( 64 )
            .Known( neonDivide );
        // Between the poles at +-pi/2 and +-3pi/2, Tan_Reduce subtracts pi and folds the sign, near +-pi the float rounding of pi is absolute error
        RegisterTest( tests, "f32 tan reduced", []( TestRegf32 a ) { return FS::Tan( a ); } ).Inputs( In::Magnitude( 1.6, 4.6 ).Specials( { pi, 3 * pi / 4, 5 * pi / 4 } ) ).RelaxedAccuracy( 128, 3.e-7f )
            .Known( neonDivide );
        RegisterTest( tests, "f32 acos", []( TestRegf32 a ) { return FS::ACos( a ); } ).Inputs( In::Range( -1, 1 ) ).RelaxedAccuracy( 8 );
        // ASin is K_HALF_PI - ACos( x ), near 0 that cancels and ACos's error (~2 ULP of pi/2) becomes absolute error, +0 for |x| <= 2^-25
        RegisterTest( tests, "f32 asin", []( TestRegf32 a ) { return FS::ASin( a ); } ).Inputs( In::Range( -1, 1 ) ).RelaxedAccuracy( 32, 3.e-7f );
        // Relaxed x86 ATan uses the rcpps estimate for |x| > 1, its 1.5 * 2^-12 relative error is up to ~3072 ULP of atan near 1
        // Relaxed scalar is std::atan, so FS::ATan itself is checked much more closely
        RegisterTest( tests, "f32 atan", []( TestRegf32 a ) { return FS::ATan( a ); } ).Inputs( In::NoNaN() ).RelaxedAccuracy( 4096 ).ScalarRelaxedAccuracy( 64 )
            .Known( neonDivide );
        RegisterTest( tests, "f32 atan2", []( TestRegf32 a, TestRegf32 b ) { return FS::ATan2( a, b ); } ).Inputs( In::NoNaN(), In::NoNaN() ).RelaxedAccuracy( 4096 ).ScalarRelaxedAccuracy( 64 )
            .Known( KnownFailure( "Relaxed scalar uses std::atan2: for atan2( +-0, -0 ) it returns +-pi where FS::ATan2 returns 0, and for atan2( +-inf, +-inf ) +-pi/4 or +-3pi/4 where FS::ATan2 returns NaN (inf / inf)",
                                  Tiers::Relaxed, FeatureSet::SCALAR, FeatureSet::SCALAR ),
                    neonDivide, neonDivideDenormal );
        // Keep relaxed scalar checking FS::ATan2 against std::atan2 everywhere except both inputs 0
        RegisterTest( tests, "f32 atan2 nonzero y", []( TestRegf32 a, TestRegf32 b ) { return FS::ATan2( a, b ); } ).Inputs( In::Magnitude( denormMin, floatMax ), In::NoNaN() ).RelaxedAccuracy( 4096 ).ScalarRelaxedAccuracy( 64 )
            .Known( neonDivide, neonDivideDenormal );
        RegisterTest( tests, "f32 atan2 nonzero x", []( TestRegf32 a, TestRegf32 b ) { return FS::ATan2( a, b ); } ).Inputs( In::NoNaN(), In::Magnitude( denormMin, floatMax ) ).RelaxedAccuracy( 4096 ).ScalarRelaxedAccuracy( 64 )
            .Known( neonDivide, neonDivideDenormal );
        // FS::Exp clamps its input to +-88.3762626647949, the range reaches just below that and keeps results above FLT_MIN
        RegisterTest( tests, "f32 exp", []( TestRegf32 a ) { return FS::Exp( a ); } ).Inputs( In::Range( -87.3, 88.37 ) ).RelaxedAccuracy( 8 );
        RegisterTest( tests, "f32 exp2", []( TestRegf32 a ) { return FS::Exp2( a ); } ).Inputs( In::Range( -126, 127 ) ).RelaxedAccuracy( 8 );
        RegisterTest( tests, "f32 log", []( TestRegf32 a ) { return FS::Log( a ); } ).Inputs( In::Range( 1.e-20, 1.e+20 ) ).RelaxedAccuracy( 8 );
        // Log2 reduces the mantissa to [1,2), just below 1 it cancels to ~2e-7 absolute error
        RegisterTest( tests, "f32 log2", []( TestRegf32 a ) { return FS::Log2( a ); } ).Inputs( In::Range( 1.e-20, 1.e+20 ) ).RelaxedAccuracy( 8, 3.e-7f );
        // Results stay within float range, FS::Pow saturates where std::pow returns inf
        // Pow is Exp( b * Log( a ) ), so Log's relative error is scaled by b * ln( a ), up to ~74 here
        RegisterTest( tests, "f32 pow", []( TestRegf32 a, TestRegf32 b ) { return FS::Pow( a, b ); } )
            .Inputs( In::Range( 1.e-4, 1.e+4 ), In::Range( -8, 8 ) ).RelaxedAccuracy( 256 );
        RegisterTest( tests, "f32 pow zero base", []( TestRegf32 b ) { return FS::Pow( TestRegf32( 0.0f ), b ); }, []( float ) { return 0.0f; } ).Inputs( In::Range( 0.1, 8 ) );

        RegisterTest( tests, "f32 inv sqrt full range", []( TestRegf32 a ) { return FS::InvSqrt( a ); }, []( float a ) { return 1.0f / std::sqrt( a ); } ).RelaxedAccuracy( 8192 )
            .Known( neonDivide, x86EstimateDenormals, armEstimateSpecials );
        RegisterTest( tests, "f32 reciprocal full range", []( TestRegf32 a ) { return FS::Reciprocal( a ); }, []( float a ) { return 1.0f / a; } ).RelaxedAccuracy( 8192 )
            .Known( neonDivide, x86ReciprocalEstimate, armEstimateSpecials );

        // Relaxed accuracy is only meaningful over the ranges above, non relaxed results must still match exactly over the whole domain
        if constexpr( !FastSIMD::IsRelaxed() )
        {
            RegisterTest( tests, "f32 min full range", []( TestRegf32 a, TestRegf32 b ) { return FS::Min( a, b ); }, []( float a, float b ) { return std::min( a, b ); } );
            RegisterTest( tests, "f32 max full range", []( TestRegf32 a, TestRegf32 b ) { return FS::Max( a, b ); }, []( float a, float b ) { return std::max( a, b ); } );
            RegisterTest( tests, "f32 cos full range", []( TestRegf32 a ) { return FS::Cos( a ); } );
            RegisterTest( tests, "f32 sin full range", []( TestRegf32 a ) { return FS::Sin( a ); } )
                .Known( sinReduceZeroSign );
            RegisterTest( tests, "f32 tan full range", []( TestRegf32 a ) { return FS::Tan( a ); } )
                .Known( neonDivide );
            // Exp clamps its input, no NaN since converting it to int is undefined in the scalar reference
            RegisterTest( tests, "f32 exp full range", []( TestRegf32 a ) { return FS::Exp( a ); } )
                .Inputs( In::NoNaN().Specials( { 88.3762626647949, -88.3762626647949, 88.72283935546875, -87.33654022216797, -103.97207641601562 } ) );
            RegisterTest( tests, "f32 log full range", []( TestRegf32 a ) { return FS::Log( a ); } ).Inputs( In::Range( denormMin, floatMax ) );
            RegisterTest( tests, "f32 log2 full range", []( TestRegf32 a ) { return FS::Log2( a ); } ).Inputs( In::Range( denormMin, floatMax ) );
            RegisterTest( tests, "f32 pow full range", []( TestRegf32 a, TestRegf32 b ) { return FS::Pow( a, b ); } ).Inputs( In::Range( denormMin, floatMax ), In::Finite() );
        }
    }

    void RegisterConvertTests( TestCollection& tests )
    {
        RegisterTest( tests, "i32 convert to f32", []( TestRegi32 a ) { return FS::Convert<float>( a ); }, []( int32_t a ) { return static_cast<float>( a ); } );
        RegisterTest( tests, "f32 convert to i32", []( TestRegf32 a ) { return FS::Convert<int32_t>( a ); }, []( float a ) { return static_cast<int32_t>( std::rint( a ) ); } )
            .Inputs( In::Range( -2147483648.0, 2147483520.0 ) );

        RegisterTest( tests, "f32 cast to i32", []( TestRegf32 a ) { return FS::Cast<int32_t>( a ); }, []( float a ) { return static_cast<int32_t>( ToBits( a ) ); } );
        RegisterTest( tests, "i32 cast to f32", []( TestRegi32 a ) { return FS::Cast<float>( a ); }, []( int32_t a ) { return U( a ); } ).BitExact();

        // Cast is a raw reinterpret, a mask's bits as another type differ between SIMD levels (and k-registers can't be cast)
        // Only the round trip back to the mask is defined, so that's all these test
        if constexpr( !( SIMD & FeatureFlag::AVX512_F ) || RegisterBytes < 64 )
        {
            RegisterTest( tests, "m32 cast to i32", []( TestRegm32 a ) { return FS_BIND_INTRINSIC( FS::Cast<FS::Mask<32>> )( FS_BIND_INTRINSIC( FS::Cast<int32_t> )( a ) ); }, []( bool a ) { return a; } );
            RegisterTest( tests, "m32 cast to f32", []( TestRegm32 a ) { return FS_BIND_INTRINSIC( FS::Cast<FS::Mask<32>> )( FS_BIND_INTRINSIC( FS::Cast<float> )( a ) ); }, []( bool a ) { return a; } );
            RegisterTest( tests, "m32i cast to i32", []( TestRegm32i a ) { return FS_BIND_INTRINSIC( FS::Cast<typename TestRegm32i::ElementType> )( FS_BIND_INTRINSIC( FS::Cast<int32_t> )( a ) ); }, []( bool a ) { return a; } );
            RegisterTest( tests, "m32i cast to f32", []( TestRegm32i a ) { return FS_BIND_INTRINSIC( FS::Cast<typename TestRegm32i::ElementType> )( FS_BIND_INTRINSIC( FS::Cast<float> )( a ) ); }, []( bool a ) { return a; } );
        }
    }
};

// Each register size is built as its own dispatch library, see tests/CMakeLists.txt
template class FastSIMD::RegisterDispatchClass<TestFastSIMD<FASTSIMD_TEST_REGISTER_BYTES, FastSIMD::IsRelaxed()>>;
