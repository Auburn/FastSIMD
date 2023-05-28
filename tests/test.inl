#pragma once
#include <bitset>

#include "test.h"

#include <functional>
#include <iomanip>
#include <iterator>
#include <sstream>

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
struct GetReturn<bool>
{
    static constexpr auto Type = TestData::ReturnType::boolean;
};

template<size_t N, bool B>
struct GetReturn<FS::Mask<N, B>>
{
    static constexpr auto Type = TestData::ReturnType::boolean;
};

template<typename T, size_t N, FastSIMD::FeatureSet S>
struct GetReturn<FS::Register<T, N, S>>
{
    static constexpr auto Type = GetReturn<T>::Type;
};

template<typename F>
static const size_t GetReturnCount = 1;

template<typename T, size_t N, FastSIMD::FeatureSet S>
static const size_t GetReturnCount<FS::Register<T, N, S>> = N;


template<FastSIMD::FeatureSet SIMD, size_t RegisterBytes>
class FastSIMD::DispatchClass<TestFastSIMD<RegisterBytes>, SIMD> : public TestFastSIMD<RegisterBytes>
{
    template<typename T>
    using TestReg = FS::Register<T, RegisterBytes / sizeof( T )>;


    template<typename F, typename Ret, typename... Args>
    static std::tuple<Args...> GetArg_Helper( Ret ( F::* )( Args... ) const );

    template<typename F, size_t I>
    using GetArg = std::tuple_element_t<I, decltype( GetArg_Helper( &F::operator() ) )>;



    template<typename T>
    static std::string AsString( T arg )
    {
        std::stringstream ss;
        ss << std::setprecision( 16 ) << std::boolalpha << arg;
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
        FS::Store( reinterpret_cast<T*>( outResult ), result );
        return N;
    }

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    static size_t StoreOutput( void* outResult, FS::Register<FS::Mask<MN, MB>, N, S> result )
    {
        bool* boolOut = reinterpret_cast<bool*>( outResult );
        auto bitMask = FS::BitMask( result );

        for( size_t idx = 0; idx < N; idx++ )
        {
            *boolOut++ = bitMask & 1;
            bitMask >>= 1;
        }    
        return N;
    }


    template<typename T>
    struct GenArg
    {
        template<typename... ARGs>
        static T Load( size_t inIdx, size_t argIdx, T* rnd, ARGs... )
        {
            return *( rnd + inIdx + argIdx );
        }

        template<typename U, typename... ARGs>
        static T Load( size_t inIdx, size_t argIdx, U, ARGs... args )
        {
            return Load( inIdx, argIdx, args... );
        }
    };

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct GenArg<FS::Register<T, N, S>>
    {
        template<typename... ARGs>
        static FS::Register<T, N, S> Load( size_t inIdx, size_t argIdx, T* rnd, ARGs... )
        {
            return FS::Load<N>( rnd + inIdx + argIdx * N );
        }

        template<typename U, typename... ARGs>
        static FS::Register<T, N, S> Load( size_t inIdx, size_t argIdx, U, ARGs... args )
        {
            return Load( inIdx, argIdx, args... );
        }
    };

    template<size_t MN, bool MB, size_t N, FastSIMD::FeatureSet S>
    struct GenArg<FS::Register<FS::Mask<MN, MB>, N, S>>
    {
        template<typename... ARGs>
        static FS::Register<FS::Mask<MN, MB>, N, S> Load( size_t inIdx, size_t argIdx, float* rnd, ARGs... )
        {
            return FS::Load<N>( rnd + inIdx + argIdx * N ) > FS::Load<N>( rnd + inIdx + argIdx * N + 1 );
        }

        template<typename U, typename... ARGs>
        static FS::Register<FS::Mask<MN, MB>, N, S> Load( size_t inIdx, size_t argIdx, U, ARGs... args )
        {
            return Load( inIdx, argIdx, args... );
        }
    };


    template <class FUNC, class = decltype(pack_helper(std::declval<FUNC>()))>
    struct TestFunctionFactory;

    template <class FUNC, class... ARGs>
    struct TestFunctionFactory<FUNC, pack<ARGs...>>
    {
        using ReturnType = std::invoke_result_t<FUNC, ARGs...>;

        static std::function<TestFunction> Create( FUNC func )
        {
            return [func]( void* outResult, size_t inIdx, int32_t* rndInts, float* rndFloats )
            {
                size_t argIdx = 0;
                auto tuple = std::tuple{ GenArg<std::decay_t<ARGs>>::Load( inIdx, argIdx++, rndInts, rndFloats )... };

                return StoreOutput( outResult, std::apply( func, tuple ) );
            };
        }

        static std::function<InputsFunction> GetInputs()
        {
            return []( size_t inIdx, int32_t* rndInts, float* rndFloats ) -> std::string
            {
                if( !sizeof...( ARGs ) )
                {
                    return "No Inputs";
                }

                size_t argIdx = 0;
                std::initializer_list<std::string> tuple = { AsString( GenArg<std::decay_t<ARGs>>::Load( inIdx, argIdx++, rndInts, rndFloats ) )... };

                std::ostringstream inputsString;

                std::copy( tuple.begin(), tuple.end() - 1, std::ostream_iterator<std::string>( inputsString, ", " ) );                

                inputsString << *(tuple.end() - 1);

                return inputsString.str();
            };
        }
    };


    template<typename FUNC>
    void RegisterTest( TestCollection& testMap, const char* name, FUNC func )
    {
        TestData data;
        data.featureSet = SIMD;
        data.returnType = GetReturn<typename TestFunctionFactory<FUNC>::ReturnType>::Type;

        data.testFunc = TestFunctionFactory<FUNC>::Create( func );
        data.inputsFunc = TestFunctionFactory<FUNC>::GetInputs();

        testMap.emplace_back( name, data );
    }

    template<typename... ARGS, typename F>
    auto make_func( F func )
    {
        return std::function<std::invoke_result_t<F( ARGS... )>( ARGS... )>( static_cast<std::invoke_result_t<F( ARGS... )>( ARGS... )>( func ) );
    }

    TestCollection RegisterTests() override
    {
        TestCollection tests;

        using TestRegi32 = TestReg<int32_t>;
        using TestRegf32 = TestReg<float>;
        using TestRegm32 = typename TestRegf32::MaskType;


        RegisterTest( tests, "m32 bit mask", []( TestRegm32 a ) { return a; } );
        RegisterTest( tests, "m32 any mask", []( TestRegm32 a ) { return FS::AnyMask( a ); } );

        RegisterTest( tests, "m32 bit and operator", std::bit_and<TestRegm32>() );
        RegisterTest( tests, "m32 bit or operator", std::bit_or<TestRegm32>() );
        RegisterTest( tests, "m32 bit xor operator", std::bit_xor<TestRegm32>() );
        RegisterTest( tests, "m32 bit not operator", std::bit_not<TestRegm32>() );
        RegisterTest( tests, "m32 bit and not", []( TestRegm32 a, TestRegm32 b ) { return FS::BitwiseAndNot( a, b ); } );

        RegisterTest( tests, "i32 load store", []( TestRegi32 a ) { return a; } );
        RegisterTest( tests, "i32 load scalar", []( int32_t a ) { return TestRegi32( a ); } );
        RegisterTest( tests, "i32 splat", []( int32_t a ) { return FS::Splat<TestRegi32::ElementCount>( a ); } );
        RegisterTest( tests, "i32 extract 0", []( TestRegi32 a ) { return FS::Extract0( a ); } );        
        RegisterTest( tests, "i32 load incremented", []() { return FS::LoadIncremented<TestRegi32>(); } );
        
        RegisterTest( tests, "i32 plus operator", std::plus<TestRegi32>() );
        RegisterTest( tests, "i32 minus operator", std::minus<TestRegi32>() );
        RegisterTest( tests, "i32 multiply operator", std::multiplies<TestRegi32>() );
        
        RegisterTest( tests, "i32 bit and operator", std::bit_and<TestRegi32>() );
        RegisterTest( tests, "i32 bit or operator", std::bit_or<TestRegi32>() );
        RegisterTest( tests, "i32 bit xor operator", std::bit_xor<TestRegi32>() );
        RegisterTest( tests, "i32 bit not operator", std::bit_not<TestRegi32>() );
        RegisterTest( tests, "i32 negate operator", std::negate<TestRegi32>() );
        RegisterTest( tests, "i32 bit and not", []( TestRegi32 a, TestRegi32 b ) { return FS::BitwiseAndNot( a, b ); } );
        
        RegisterTest( tests, "i32 increment", []( TestRegi32 a ) { return FS::Increment( a ); } );
        RegisterTest( tests, "i32 decrement", []( TestRegi32 a ) { return FS::Decrement( a ); } );
        RegisterTest( tests, "i32 abs", []( TestRegi32 a ) { return FS::Abs( a ); } );
        RegisterTest( tests, "i32 min", []( TestRegi32 a, TestRegi32 b ) { return FS::Min( a, b ); } );
        RegisterTest( tests, "i32 max", []( TestRegi32 a, TestRegi32 b ) { return FS::Max( a, b ); } );
        
        RegisterTest( tests, "i32 bit shift left scalar", []( TestRegi32 a, int b ) { return a << ( b & 31 ); } );
        RegisterTest( tests, "i32 bit shift right scalar", []( TestRegi32 a, int b ) { return a >> ( b & 31 ); } );
        RegisterTest( tests, "i32 bit shift right zero extend scalar", []( TestRegi32 a, int b ) { return FS::BitShiftRightZeroExtend( a, b & 31 ); } );
        
        //RegisterTest( tests, "i32 bit shift left", []( TestRegi32 a, TestRegi32 b ) { return a << FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );
        //RegisterTest( tests, "i32 bit shift right", []( TestRegi32 a, TestRegi32 b ) { return a >> FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );
        
        RegisterTest( tests, "i32 equals operator", []( TestRegi32 a, TestRegi32 b ) { return a == b; } );
        RegisterTest( tests, "i32 equals operator alt", []( TestRegi32 a ) { return a == a; } );
        RegisterTest( tests, "i32 not equals operator", []( TestRegi32 a, TestRegi32 b ) { return a != b; } );
        RegisterTest( tests, "i32 not equals operator alt", []( TestRegi32 a ) { return a != a; } );
        RegisterTest( tests, "i32 less than operator", []( TestRegi32 a, TestRegi32 b ) { return a < b; } );
        RegisterTest( tests, "i32 greater than operator", []( TestRegi32 a, TestRegi32 b ) { return a > b; } );
        RegisterTest( tests, "i32 less equal than operator", []( TestRegi32 a, TestRegi32 b ) { return a <= b; } );
        RegisterTest( tests, "i32 greater equal than operator", []( TestRegi32 a, TestRegi32 b ) { return a >= b; } );
        
        RegisterTest( tests, "i32 select", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::Select( m, a, b ); } );
        RegisterTest( tests, "i32 masked", []( TestRegm32 m, TestRegi32 a ) { return FS::Masked( m, a ); } );
        RegisterTest( tests, "i32 inv masked", []( TestRegm32 m, TestRegi32 a ) { return FS::InvMasked( m, a ); } );
        RegisterTest( tests, "i32 masked increment", []( TestRegm32 m, TestRegi32 a ) { return FS::MaskedIncrement( m, a ); } );
        RegisterTest( tests, "i32 masked decrement", []( TestRegm32 m, TestRegi32 a ) { return FS::MaskedDecrement( m, a ); } );
        RegisterTest( tests, "i32 masked add", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::MaskedAdd( m, a, b ); } );
        RegisterTest( tests, "i32 masked sub", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::MaskedSub( m, a, b ); } );
        RegisterTest( tests, "i32 inv masked add", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::InvMaskedAdd( m, a, b ); } );
        RegisterTest( tests, "i32 inv masked sub", []( TestRegm32 m, TestRegi32 a, TestRegi32 b ) { return FS::InvMaskedSub( m, a, b ); } );
        
        RegisterTest( tests, "f32 load store", []( TestRegf32 a ) { return a; } );
        RegisterTest( tests, "f32 load scalar", []( float a ) { return TestRegf32( a ); } );
        RegisterTest( tests, "f32 splat", []( float a ) { return FS::Splat<TestRegf32::ElementCount>( a ); } );
        RegisterTest( tests, "f32 extract 0", []( TestRegf32 a ) { return FS::Extract0( a ); } );
        RegisterTest( tests, "f32 load incremented", []() { return FS::LoadIncremented<TestRegf32>(); } );
        
        RegisterTest( tests, "f32 plus operator", std::plus<TestRegf32>() );
        RegisterTest( tests, "f32 minus operator", std::minus<TestRegf32>() );
        RegisterTest( tests, "f32 multiply operator", std::multiplies<TestRegf32>() );
        RegisterTest( tests, "f32 divide operator", std::divides<TestRegf32>() );

        RegisterTest( tests, "f32 fused multiply add", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FMulAdd( a, b, c ); } );
        RegisterTest( tests, "f32 fused negative multiply add", []( TestRegf32 a, TestRegf32 b, TestRegf32 c ) { return FS::FNMulAdd( a, b, c ); } );
        
        RegisterTest( tests, "f32 bit and operator", std::bit_and<TestRegf32>() );
        RegisterTest( tests, "f32 bit or operator", std::bit_or<TestRegf32>() );
        RegisterTest( tests, "f32 bit xor operator", std::bit_xor<TestRegf32>() );
        RegisterTest( tests, "f32 bit not operator", std::bit_not<TestRegf32>() );
        RegisterTest( tests, "f32 negate operator", std::negate<TestRegf32>() );
        RegisterTest( tests, "f32 bit and not", []( TestRegf32 a, TestRegf32 b ) { return FS::BitwiseAndNot( a, b ); } );
        
        RegisterTest( tests, "f32 equals operator", []( TestRegf32 a, TestRegf32 b ) { return a == b; } );
        RegisterTest( tests, "f32 greater equal than operator", []( TestRegf32 a, TestRegf32 b ) { return a >= b; } );
        RegisterTest( tests, "f32 not equals operator", []( TestRegf32 a, TestRegf32 b ) { return a != b; } );
        RegisterTest( tests, "f32 less than operator", []( TestRegf32 a, TestRegf32 b ) { return a < b; } );
        RegisterTest( tests, "f32 greater than operator", []( TestRegf32 a, TestRegf32 b ) { return a > b; } );
        RegisterTest( tests, "f32 less equal than operator", []( TestRegf32 a, TestRegf32 b ) { return a <= b; } );

        RegisterTest( tests, "f32 increment", []( TestRegf32 a ) { return FS::Increment( a ); } );
        RegisterTest( tests, "f32 decrement", []( TestRegf32 a ) { return FS::Decrement( a ); } );
        RegisterTest( tests, "f32 abs", []( TestRegf32 a ) { return FS::Abs( a ); } );
        RegisterTest( tests, "f32 min", []( TestRegf32 a, TestRegf32 b ) { return FS::Min( a, b ); } );
        RegisterTest( tests, "f32 max", []( TestRegf32 a, TestRegf32 b ) { return FS::Max( a, b ); } );

        RegisterTest( tests, "f32 select", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::Select( m, a, b ); } );
        RegisterTest( tests, "f32 masked", []( TestRegm32 m, TestRegf32 a ) { return FS::Masked( m, a ); } );
        RegisterTest( tests, "f32 inv masked", []( TestRegm32 m, TestRegf32 a ) { return FS::InvMasked( m, a ); } );
        RegisterTest( tests, "f32 masked increment", []( TestRegm32 m, TestRegf32 a ) { return FS::MaskedIncrement( m, a ); } );
        RegisterTest( tests, "f32 masked decrement", []( TestRegm32 m, TestRegf32 a ) { return FS::MaskedDecrement( m, a ); } );
        RegisterTest( tests, "f32 masked add", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::MaskedAdd( m, a, b ); } );
        RegisterTest( tests, "f32 masked sub", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::MaskedSub( m, a, b ); } );
        RegisterTest( tests, "f32 inv masked add", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::InvMaskedAdd( m, a, b ); } );
        RegisterTest( tests, "f32 inv masked sub", []( TestRegm32 m, TestRegf32 a, TestRegf32 b ) { return FS::InvMaskedSub( m, a, b ); } );
        
        RegisterTest( tests, "f32 round", []( TestRegf32 a ) { return FS::Round( a ); } );
        RegisterTest( tests, "f32 ceil", []( TestRegf32 a ) { return FS::Ceil( a ); } );
        RegisterTest( tests, "f32 floor", []( TestRegf32 a ) { return FS::Floor( a ); } );

        RegisterTest( tests, "f32 reciprocal", []( TestRegf32 a ) { return FS::Reciprocal( a ); } );
        RegisterTest( tests, "f32 inv sqrt", []( TestRegf32 a ) { return FS::InvSqrt( a ); } );

        RegisterTest( tests, "f32 cos", []( TestRegf32 a ) { return FS::Cos( a ); } );
        RegisterTest( tests, "f32 sin", []( TestRegf32 a ) { return FS::Sin( a ); } );
        RegisterTest( tests, "f32 exp", []( TestRegf32 a ) { return FS::Exp( a ); } );
        RegisterTest( tests, "f32 log", []( TestRegf32 a ) { return FS::Log( a ); } );
        RegisterTest( tests, "f32 pow", []( TestRegf32 a, TestRegf32 b ) { return FS::Pow( a, b ); } );

        RegisterTest( tests, "i32 convert to f32", []( TestRegi32 a ) { return FS::Convert<float>( a ); } );
        RegisterTest( tests, "i32 cast to f32", []( TestRegi32 a ) { return FS::Cast<float>( a ); } );
        RegisterTest( tests, "f32 convert to i32", []( TestRegf32 a ) { return FS::Convert<int32_t>( FS::Min( FS::Max( a, TestRegf32( INT32_MIN ) ), TestRegf32( -(float)INT32_MIN ) ) ); } );
        RegisterTest( tests, "f32 cast to i32", []( TestRegf32 a ) { return FS::Cast<int32_t>( a ); } );

        return tests;
    }
};

template class FastSIMD::RegisterDispatchClass<TestFastSIMD<256 / 8>>;