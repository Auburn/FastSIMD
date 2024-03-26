#include <FastSIMD/ToolSet.h>

int main()
{
    auto distSq = FS::LoadIncremented<FS::f32<4>>();

    auto invSqrt = FS::InvSqrt( distSq );

    auto dist = invSqrt * distSq;

    auto out = FS::Masked( invSqrt != FS::f32<4>( INFINITY ), dist );

    return FS::Extract0( FS::Convert<int>( out ) );
}
