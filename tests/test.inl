#pragma once
#include "test.h"

#include <functional>
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


template<FastSIMD::FeatureSet SIMD, size_t RegisterBytes>
class FastSIMD::DispatchClass<TestFastSIMD<RegisterBytes>, SIMD> : public TestFastSIMD<RegisterBytes>
{
    template<typename T>
    using TestReg = FS::Register<T, RegisterBytes / sizeof( T )>;

    template<typename F, typename Ret, typename... Args>
    static std::tuple<Args...> GetArg_Helper( Ret ( F::* )( Args... ) const );

    template<typename F, size_t I>
    using GetArg = std::tuple_element_t<I, decltype( GetArg_Helper( &F::operator() ) )>;

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

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    struct GetReturn<FS::Register<T, N, S>>
    {
        static constexpr auto Type = GetReturn<T>::Type;
    };


    template<typename F>
    static const size_t GetReturnCount = 1;

    template<typename T, size_t N, FastSIMD::FeatureSet S>
    static const size_t GetReturnCount<FS::Register<T, N, S>> = N;

    template<typename T>
    static std::string AsString( T arg )
    {
        return std::to_string( arg );
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
                if( !sizeof...(ARGs) )
                {
                    return "No Inputs";
                }

                size_t argIdx = 0;
                auto tuple = { AsString( GenArg<std::decay_t<ARGs>>::Load( inIdx, argIdx++, rndInts, rndFloats ) )... };

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

    TestCollection RegisterTests() override
    {
        TestCollection tests;

        using TestRegi32 = TestReg<int32_t>;
        using TestRegf32 = TestReg<float>;


        RegisterTest( tests, "i32 load store", []( TestRegi32 a ) { return a; } );

        RegisterTest( tests, "i32 plus operator", std::plus<TestRegi32>() );
        RegisterTest( tests, "i32 minus operator", std::minus<TestRegi32>() );
        RegisterTest( tests, "i32 multiply operator", std::multiplies<TestRegi32>() );

        RegisterTest( tests, "i32 bit and operator", std::bit_and<TestRegi32>() );
        RegisterTest( tests, "i32 bit or operator", std::bit_or<TestRegi32>() );
        RegisterTest( tests, "i32 bit xor operator", std::bit_xor<TestRegi32>() );
        RegisterTest( tests, "i32 bit not operator", std::bit_not<TestRegi32>() );
        RegisterTest( tests, "i32 negate operator", std::negate<TestRegi32>() );

        RegisterTest( tests, "i32 Abs", []( TestRegi32 a ) { return FS::Abs( a ); } );
        
        RegisterTest( tests, "i32 equals operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a == b, a, b ); } ); 

        RegisterTest( tests, "i32 bit shift left scalar", []( TestRegi32 a, int b ) { return a << std::min( 31, std::abs( b ) ); } );
        RegisterTest( tests, "i32 bit shift right scalar", []( TestRegi32 a, int b ) { return a >> std::min( 31, std::abs( b ) ); } );

        //RegisterTest( tests, "i32 bit shift left", []( TestRegi32 a, TestRegi32 b ) { return a << FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );
        //RegisterTest( tests, "i32 bit shift right", []( TestRegi32 a, TestRegi32 b ) { return a >> FS::Min( TestRegi32( 31 ), FS::Abs( b ) ); } );

        RegisterTest( tests, "i32 equals operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a == b, a, b ); } );
        RegisterTest( tests, "i32 equals operator alt", []( TestRegi32 a ) { return FS::Select( a == a, a, TestRegi32( 0 ) ); } );
        RegisterTest( tests, "i32 not equals operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a != b, a, b ); } );
        RegisterTest( tests, "i32 not equals operator alt", []( TestRegi32 a ) { return FS::Select( a != a, a, TestRegi32( 0 ) ); } );
        RegisterTest( tests, "i32 less than operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a < b, a, b ); } );
        RegisterTest( tests, "i32 greater than operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a > b, a, b ); } );
        RegisterTest( tests, "i32 less equal than operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a <= b, a, b ); } );
        RegisterTest( tests, "i32 greater equal than operator", []( TestRegi32 a, TestRegi32 b ) { return FS::Select( a >= b, a, b ); } );


        RegisterTest( tests, "f32 load store", []( TestRegf32 a ) { return a; } );

        RegisterTest( tests, "f32 plus operator", std::plus<TestRegf32>() );
        RegisterTest( tests, "f32 minus operator", std::minus<TestRegf32>() );
        RegisterTest( tests, "f32 multiply operator", std::multiplies<TestRegf32>() );
        RegisterTest( tests, "f32 divide operator", std::divides<TestRegf32>() );

        RegisterTest( tests, "f32 bit and operator", std::bit_and<TestRegf32>() );
        RegisterTest( tests, "f32 bit or operator", std::bit_or<TestRegf32>() );
        RegisterTest( tests, "f32 bit xor operator", std::bit_xor<TestRegf32>() );
        RegisterTest( tests, "f32 bit not operator", std::bit_not<TestRegf32>() );
        RegisterTest( tests, "f32 negate operator", std::negate<TestRegf32>() );

        RegisterTest( tests, "f32 equals operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a == b, a, b ); } );
        RegisterTest( tests, "f32 not equals operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a != b, a, b ); } );
        RegisterTest( tests, "f32 less than operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a < b, a, b ); } );
        RegisterTest( tests, "f32 greater than operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a > b, a, b ); } );
        RegisterTest( tests, "f32 less equal than operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a <= b, a, b ); } );
        RegisterTest( tests, "f32 greater equal than operator", []( TestRegf32 a, TestRegf32 b ) { return FS::Select( a >= b, a, b ); } );

        RegisterTest( tests, "f32 Abs", []( TestRegf32 a ) { return FS::Abs( a ); } );
        RegisterTest( tests, "f32 Round", []( TestRegf32 a ) { return FS::Round( a ); } );
        RegisterTest( tests, "f32 Ceil", []( TestRegf32 a ) { return FS::Ceil( a ); } );
        RegisterTest( tests, "f32 Floor", []( TestRegf32 a ) { return FS::Floor( a ); } );

        return tests;
    }
};

template class FastSIMD::RegisterDispatchClass<TestFastSIMD<128 / 8>>;