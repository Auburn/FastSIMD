#pragma once

#include <algorithm>
#include <cmath>

#include "Register.h"
#include "Functions.h"

namespace FS
{
    namespace impl 
    {
        // Trigonometric and mathematical constants
        template<typename T>
        struct TrigConstants
        {
            static constexpr T K_PI          = (T)3.1415926535897931;
            static constexpr T K_HALF_PI     = (T)1.5707963267948966;
            static constexpr T K_QUARTER_PI  = (T)0.7853981633974483;
            static constexpr T K_TWO_PI      = (T)6.2831853071795862;
            static constexpr T K_INV_PI      = (T)0.3183098861837907;
            static constexpr T K_INV_TWO_PI  = (T)0.1591549430918953;
            static constexpr T K_INV_HALF_PI = (T)0.6366197723675813;
            static constexpr T K_INV_LN_2    = (T)0.6931471805599453;
            static constexpr T K_LN_2        = (T)1.4426950408889634;
            
            // ACOS polynomial coefficients
            static constexpr T ACOS_DEG8_C0  = (T)+1.5707963267948966;
            static constexpr T ACOS_DEG8_C1  = (T)-2.1460143648688035e-01;
            static constexpr T ACOS_DEG8_C2  = (T)+8.9034700107934128e-02;
            static constexpr T ACOS_DEG8_C3  = (T)-5.0625279962389413e-02;
            static constexpr T ACOS_DEG8_C4  = (T)+3.2683762943179318e-02;
            static constexpr T ACOS_DEG8_C5  = (T)-2.0949278766238422e-02;
            static constexpr T ACOS_DEG8_C6  = (T)+1.1272900916992512e-02;
            static constexpr T ACOS_DEG8_C7  = (T)-4.1160981058965262e-03;
            static constexpr T ACOS_DEG8_C8  = (T)+7.1796493341480527e-04;
            
            // ATAN polynomial coefficients
            static constexpr T ATAN_DEG13_C0 = (T)+1.0;
            static constexpr T ATAN_DEG13_C1 = (T)-3.3324998579202170e-01;
            static constexpr T ATAN_DEG13_C2 = (T)+1.9856563505717162e-01;
            static constexpr T ATAN_DEG13_C3 = (T)-1.3374657325451267e-01;
            static constexpr T ATAN_DEG13_C4 = (T)+8.1675882859940430e-02;
            static constexpr T ATAN_DEG13_C5 = (T)-3.5059680836411644e-02;
            static constexpr T ATAN_DEG13_C6 = (T)+7.2128853633444123e-03;
            
            // COS polynomial coefficients
            static constexpr T COS_DEG10_C0  = (T)+1.0;
            static constexpr T COS_DEG10_C1  = (T)-4.9999999508695869e-01;
            static constexpr T COS_DEG10_C2  = (T)+4.1666638865338612e-02;
            static constexpr T COS_DEG10_C3  = (T)-1.3888377661039897e-03;
            static constexpr T COS_DEG10_C4  = (T)+2.4760495088926859e-05;
            static constexpr T COS_DEG10_C5  = (T)-2.6051615464872668e-07;
            
            // SIN polynomial coefficients
            static constexpr T SIN_DEG11_C0  = (T)+1.0;
            static constexpr T SIN_DEG11_C1  = (T)-1.6666666601721269e-01;
            static constexpr T SIN_DEG11_C2  = (T)+8.3333303183525942e-03;
            static constexpr T SIN_DEG11_C3  = (T)-1.9840782426250314e-04;
            static constexpr T SIN_DEG11_C4  = (T)+2.7521557770526783e-06;
            static constexpr T SIN_DEG11_C5  = (T)-2.3828544692960918e-08;
            
            // TAN polynomial coefficients
            static constexpr T TAN_DEG13_C0  = (T)1.0;
            static constexpr T TAN_DEG13_C1  = (T)3.3332916426394554e-01;
            static constexpr T TAN_DEG13_C2  = (T)1.3343404625112498e-01;
            static constexpr T TAN_DEG13_C3  = (T)5.3104565343119248e-02;
            static constexpr T TAN_DEG13_C4  = (T)2.5355038312682154e-02;
            static constexpr T TAN_DEG13_C5  = (T)1.8253255966556026e-03;
            static constexpr T TAN_DEG13_C6  = (T)1.0069407176615641e-02;
            
            // LOG2 polynomial coefficients
            static constexpr T LOG2_DEG8_C1  = (T)+1.4426896453621882;
            static constexpr T LOG2_DEG8_C2  = (T)-7.2115893912535967e-01;
            static constexpr T LOG2_DEG8_C3  = (T)+4.7861716616785088e-01;
            static constexpr T LOG2_DEG8_C4  = (T)-3.4699935395019565e-01;
            static constexpr T LOG2_DEG8_C5  = (T)+2.4114048765477492e-01;
            static constexpr T LOG2_DEG8_C6  = (T)-1.3657398692885181e-01;
            static constexpr T LOG2_DEG8_C7  = (T)+5.1421382871922106e-02;
            static constexpr T LOG2_DEG8_C8  = (T)-9.1364020499895560e-03;
            
            // EXP2 polynomial coefficients
            static constexpr T EXP2_DEG7_C0  = (T)1.0;
            static constexpr T EXP2_DEG7_C1  = (T)6.9314718588750690e-01;
            static constexpr T EXP2_DEG7_C2  = (T)2.4022637363165700e-01;
            static constexpr T EXP2_DEG7_C3  = (T)5.5505235570535660e-02;
            static constexpr T EXP2_DEG7_C4  = (T)9.6136265387940512e-03;
            static constexpr T EXP2_DEG7_C5  = (T)1.3429234504656051e-03;
            static constexpr T EXP2_DEG7_C6  = (T)1.4299202757683815e-04;
            static constexpr T EXP2_DEG7_C7  = (T)2.1662892777385423e-05;
        };

        // ACos approximation for range 0:1
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> ACos_0_1( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> poly( C::ACOS_DEG8_C8 );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C7 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C6 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C5 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C4 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C3 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C2 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C1 ) );
            poly = FMulAdd( poly, x, Register<T, N, SIMD>( C::ACOS_DEG8_C0 ) );
            return poly * Sqrt( Register<T, N, SIMD>( (T)1.0 ) - x );
        }

        // ATan approximation for range -1:1
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> ATan_Neg1_1( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> xsqr = x * x;
            Register<T, N, SIMD> poly( C::ATAN_DEG13_C6 );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C5 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C4 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C3 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C2 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C1 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::ATAN_DEG13_C0 ) );
            return poly * x;
        }

        // Cosine approximation for range -pi/2:pi/2
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Cos_NegHalfPi_HalfPi( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> xsqr = x * x;
            Register<T, N, SIMD> poly( C::COS_DEG10_C5 );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::COS_DEG10_C4 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::COS_DEG10_C3 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::COS_DEG10_C2 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::COS_DEG10_C1 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::COS_DEG10_C0 ) );
            return poly;
        }

        // Reduce cosine input to appropriate range and sign
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE void Cos_Reduce( const Register<T, N, SIMD>& x, Register<T, N, SIMD>& y, Register<T, N, SIMD>& sign )
        {
            using C = TrigConstants<T>;
            // Map x to y in [-pi,pi], x = 2*pi*quotient + remainder.
            Register<T, N, SIMD> quotient = x * Register<T, N, SIMD>( C::K_INV_TWO_PI );
            Register<T, N, SIMD> offset = SignBit( x ) | Register<T, N, SIMD>( (T)0.5 );
            quotient = Trunc( quotient + offset );
            y = FMulAdd( quotient, Register<T, N, SIMD>( -C::K_TWO_PI ), x );

            // Map y to [-pi/2,pi/2] with cos(y) = sign*cos(x).
            Register<T, N, SIMD> yAbs = Abs( y );
            Register<T, N, SIMD> ySign = y ^ yAbs;
            auto gtHalfPi = yAbs > Register<T, N, SIMD>( C::K_HALF_PI );
            
            yAbs = Select( gtHalfPi, Register<T, N, SIMD>( C::K_PI ) - yAbs, yAbs );
            y = ySign ^ yAbs;
            
            sign = Select( gtHalfPi, Register<T, N, SIMD>( (T)-1.0 ), Register<T, N, SIMD>( (T)1.0 ) );
        }

        // Sine approximation for range -pi/2:pi/2
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Sin_NegHalfPi_HalfPi( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> xsqr = x * x;
            Register<T, N, SIMD> poly( C::SIN_DEG11_C5 );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::SIN_DEG11_C4 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::SIN_DEG11_C3 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::SIN_DEG11_C2 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::SIN_DEG11_C1 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::SIN_DEG11_C0 ) );
            return poly * x;
        }

        // Reduce sine input to appropriate range
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Sin_Reduce( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            
            // Map x to y in [-pi,pi], x = 2*pi*quotient + remainder.
            Register<T, N, SIMD> quotient = x * Register<T, N, SIMD>( C::K_INV_TWO_PI );
            Register<T, N, SIMD> offset = SignBit(x ) | Register<T, N, SIMD>( (T)0.5 );
            quotient = Trunc( quotient + offset );
            Register<T, N, SIMD> y = FMulAdd( quotient, Register<T, N, SIMD>( -C::K_TWO_PI ), x );

            // Map y to [-pi/2,pi/2] with sin(y) = sin(x).
            Register<T, N, SIMD> yAbs = Abs( y );
            Register<T, N, SIMD> ySign = y ^ yAbs;
            auto gtHalfPi = yAbs > Register<T, N, SIMD>( C::K_HALF_PI );
            
            yAbs = Select( gtHalfPi, Register<T, N, SIMD>( C::K_PI ) - yAbs, yAbs );
            y = ySign ^ yAbs;
            
            return y;
        }

        // Tangent approximation for range -pi/4:pi/4
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Tan_NegQuartPi_QuartPi( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> xsqr = x * x;
            Register<T, N, SIMD> poly( C::TAN_DEG13_C6 );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C5 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C4 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C3 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C2 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C1 ) );
            poly = FMulAdd( poly, xsqr, Register<T, N, SIMD>( C::TAN_DEG13_C0 ) );
            return poly * x;
        }

        // Reduce tangent input to appropriate range
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Tan_Reduce( const Register<T, N, SIMD>& x )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> y = x;
            
            // Use modulus to map to [-pi, pi] (equivalent to fmod(x, pi))
            Register<T, N, SIMD> quotient = y * Register<T, N, SIMD>( -C::K_INV_PI );
            quotient = Trunc( quotient );
            y = FMulAdd( quotient, Register<T, N, SIMD>( C::K_PI ), y );

            // Map y to [-pi/2,pi/2] with tan(y) = tan(x).
            // Note: tan(pi - x) = -tan(x), so we need to flip the sign when |y| > pi/2
            Register<T, N, SIMD> yAbs = Abs( y );
            Register<T, N, SIMD> ySign = y ^ yAbs;
            auto gtHalfPi = yAbs > Register<T, N, SIMD>( C::K_HALF_PI );
            
            yAbs = Select( gtHalfPi, Register<T, N, SIMD>( C::K_PI ) - yAbs, yAbs );
            // Flip sign when reducing from outside [-pi/2, pi/2]
            ySign = ySign ^ Masked( gtHalfPi, Register<T, N, SIMD>( (T)-0.0 ) );
            y = ySign ^ yAbs;
            
            return y;
        }

        // Log2 approximation using polynomial over (0,1]
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Log2_T_0_1( const Register<T, N, SIMD>& t )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> poly( C::LOG2_DEG8_C8 );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C7 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C6 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C5 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C4 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C3 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C2 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::LOG2_DEG8_C1 ) );
            return poly * t;
        }

        // Log2 approximation for range 1:2
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Log2_X_1_2( const Register<T, N, SIMD>& x )
        {
            Register<T, N, SIMD> t = x - Register<T, N, SIMD>( (T)1.0 );
            return Log2_T_0_1( t );
        }

        // Exp2 approximation for range 0:1
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Exp2_0_1( const Register<T, N, SIMD>& t )
        {
            using C = TrigConstants<T>;
            Register<T, N, SIMD> poly( C::EXP2_DEG7_C7 );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C6 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C5 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C4 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C3 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C2 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C1 ) );
            poly = FMulAdd( poly, t, Register<T, N, SIMD>( C::EXP2_DEG7_C0 ) );
            return poly;
        }

        // Helper function to extract mantissa and exponent for Log2
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE void Frexp( const Register<T, N, SIMD>& x, Register<T, N, SIMD>& mantissa, Register<std::int32_t, N, SIMD>& exponent )
        {
            if constexpr( std::is_same_v<T, float> )
            {
                using RegisterI = Register<std::int32_t, N, SIMD>;
                using RegisterF = Register<float, N, SIMD>;
                
                RegisterI bits = Cast<std::int32_t>( x );
                RegisterI exponentBits = BitShiftRightZeroExtend( bits & RegisterI( 0x7F800000 ), 23 );
                exponent = exponentBits - RegisterI( 126 ); // Subtract bias - 1
                
                RegisterI mantissaBits = ( bits & RegisterI( 0x007FFFFF ) ) | RegisterI( 0x3F000000 );
                mantissa = Cast<float>( mantissaBits );
            }
            else
            {
                // Fallback for other types - this would need proper implementation for double
                static_assert( std::is_same_v<T, float>, "Frexp only implemented for float" );
            }
        }

        // Helper function to scale by power of 2 for Exp2
        template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
        FS_FORCEINLINE Register<T, N, SIMD> Ldexp( const Register<T, N, SIMD>& x, const Register<std::int32_t, N, SIMD>& exp )
        {
            if constexpr( std::is_same_v<T, float> )
            {
                using RegisterI = Register<std::int32_t, N, SIMD>;
                using RegisterF = Register<float, N, SIMD>;
                
                RegisterI scaleBits = ( exp + RegisterI( 127 ) ) << 23;
                RegisterF scale = Cast<float>( scaleBits );
                return x * scale;
            }
            else
            {
                // Fallback for other types
                static_assert( std::is_same_v<T, float>, "Ldexp only implemented for float" );
            }
        }
    }

    // ACos: range -1:1
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> ACos( const Register<T, N, SIMD>& _x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            Register<T, N, SIMD> x = Abs( _x );
            Register<T, N, SIMD> result = impl::ACos_0_1( x );
            auto negMask = _x < Register<T, N, SIMD>( (T)0.0 );
            return Select( negMask, Register<T, N, SIMD>( C::K_PI ) - result, result );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::ACos not supported with provided types" );
            return Register<T, N, SIMD>{ ACos( _x.v0 ), ACos( _x.v1 ) };
        }
    }

    // ASin: range -1:1
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> ASin( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            return Register<T, N, SIMD>( C::K_HALF_PI ) - ACos( x );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::ASin not supported with provided types" );
            return Register<T, N, SIMD>{ ASin( x.v0 ), ASin( x.v1 ) };
        }
    }

    // ATan: range -inf:inf
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> ATan( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            Register<T, N, SIMD> absX = Abs( x );
            Register<T, N, SIMD> xSign = x ^ absX;
            auto leOne = absX <= Register<T, N, SIMD>( (T)1.0 );
            auto gtOne = absX > Register<T, N, SIMD>( (T)1.0 );
            
            Register<T, N, SIMD> result = impl::ATan_Neg1_1( x );
            Register<T, N, SIMD> recip = Reciprocal( x );
            Register<T, N, SIMD> atanRecip = impl::ATan_Neg1_1( recip );
            Register<T, N, SIMD> baseResult = Register<T, N, SIMD>( C::K_HALF_PI ) - atanRecip;
            Register<T, N, SIMD> resultGtOne = xSign ^ baseResult;
            
            result = Select( gtOne, resultGtOne, result );
            
            return result;
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::ATan not supported with provided types" );
            return Register<T, N, SIMD>{ ATan( x.v0 ), ATan( x.v1 ) };
        }
    }

    // ATan2: x and y can be -inf:inf, returns 0 if both 0
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> ATan2( const Register<T, N, SIMD>& y, const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            Register<T, N, SIMD> zero( (T)0.0 );
            auto xGtZero = x > zero;
            auto xLtZero = x < zero;
            auto yGtZero = y > zero;
            auto yLtZero = y < zero;
            auto yGeZero = y >= zero;
            
            Register<T, N, SIMD> ratio = y / x;
            Register<T, N, SIMD> atanRatio = ATan( ratio );
            
            // x > 0
            Register<T, N, SIMD> result = Masked( xGtZero, atanRatio );
            
            // x < 0
            Register<T, N, SIMD> signedPi = SignBit( y ) ^ Register<T, N, SIMD>( C::K_PI );
            Register<T, N, SIMD> xLtZeroResult = atanRatio + signedPi;
            result = Select( xLtZero, xLtZeroResult, result );
            
            // x == 0
            auto xEqZero = BitwiseAndNot( ~xGtZero, xLtZero );
            auto yNonZero = yGtZero | yLtZero;
            Register<T, N, SIMD> signedHalfPi = SignBit( y ) ^ Register<T, N, SIMD>( C::K_HALF_PI );
            Register<T, N, SIMD> xEqZeroResult = Masked( yNonZero, signedHalfPi );
            result = Select( xEqZero, xEqZeroResult, result );
            
            return result;
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::ATan2 not supported with provided types" );
            return Register<T, N, SIMD>{ ATan2( y.v0, x.v0 ), ATan2( y.v1, x.v1 ) };
        }
    }

    // Cos: range -inf:inf
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Cos( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            Register<T, N, SIMD> y, sign;
            impl::Cos_Reduce( x, y, sign );
            return sign * impl::Cos_NegHalfPi_HalfPi( y );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Cos not supported with provided types" );
            return Register<T, N, SIMD>{ Cos( x.v0 ), Cos( x.v1 ) };
        }
    }

    // Sin: range -inf:inf
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Sin( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            return impl::Sin_NegHalfPi_HalfPi( impl::Sin_Reduce( x ) );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Sin not supported with provided types" );
            return Register<T, N, SIMD>{ Sin( x.v0 ), Sin( x.v1 ) };
        }
    }

    // Tan: range -inf:inf
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Tan( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            Register<T, N, SIMD> y = impl::Tan_Reduce( x );
            Register<T, N, SIMD> absY = Abs( y );
            Register<T, N, SIMD> ySign = y ^ absY;
            
            auto leQuarterPi = absY <= Register<T, N, SIMD>( C::K_QUARTER_PI );
            auto gtQuarterPi = absY > Register<T, N, SIMD>( C::K_QUARTER_PI );
            
            Register<T, N, SIMD> result = impl::Tan_NegQuartPi_QuartPi( y );
            
            // |y| > K_QUARTER_PI
            Register<T, N, SIMD> poly = impl::Tan_NegQuartPi_QuartPi( absY - Register<T, N, SIMD>( C::K_QUARTER_PI ) );
            Register<T, N, SIMD> one( (T)1.0 );
            Register<T, N, SIMD> baseResult = ( one + poly ) / ( one - poly );
            Register<T, N, SIMD> resultGt = ySign ^ baseResult;
            
            result = Select( gtQuarterPi, resultGt, result );
            
            return result;
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Tan not supported with provided types" );
            return Register<T, N, SIMD>{ Tan( x.v0 ), Tan( x.v1 ) };
        }
    }

    // Log2: log2(x) for x>0
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Log2( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            Register<T, N, SIMD> mantissa;
            Register<std::int32_t, N, SIMD> exponent;
            impl::Frexp( x, mantissa, exponent );
            
            Register<T, N, SIMD> y = mantissa * Register<T, N, SIMD>( (T)2.0 );
            Register<T, N, SIMD> poly = impl::Log2_X_1_2( y );
            Register<T, N, SIMD> result = poly + Convert<T>( exponent - Register<std::int32_t, N, SIMD>( 1 ) );
            
            return result;
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Log2 not supported with provided types" );
            return Register<T, N, SIMD>{ Log2( x.v0 ), Log2( x.v1 ) };
        }
    }

    // Log: natural logarithm
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Log( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            return Log2( x ) * Register<T, N, SIMD>( C::K_INV_LN_2 );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Log not supported with provided types" );
            return Register<T, N, SIMD>{ Log( x.v0 ), Log( x.v1 ) };
        }
    }

    // Exp2: 2^x
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Exp2( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            Register<T, N, SIMD> p = Floor( x );
            Register<T, N, SIMD> y = x - p;
            Register<T, N, SIMD> poly = impl::Exp2_0_1( y );
            Register<std::int32_t, N, SIMD> exp = Convert<std::int32_t>( p );
            
            return impl::Ldexp( poly, exp );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Exp2 not supported with provided types" );
            return Register<T, N, SIMD>{ Exp2( x.v0 ), Exp2( x.v1 ) };
        }
    }

    // Exp: e^x
    template<typename T, std::size_t N, FastSIMD::FeatureSet SIMD>
    FS_FORCEINLINE Register<T, N, SIMD> Exp( const Register<T, N, SIMD>& x )
    {
        if constexpr( IsNativeV<Register<T, N, SIMD>> )
        {
            using C = impl::TrigConstants<T>;
            return Exp2( Register<T, N, SIMD>( C::K_LN_2 ) * x );
        }
        else
        {
            static_assert( !IsNativeV<Register<T, N, SIMD>>, "FastSIMD: FS::Exp not supported with provided types" );
            return Register<T, N, SIMD>{ Exp( x.v0 ), Exp( x.v1 ) };
        }
    }
}
