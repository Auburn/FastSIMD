#pragma once
#include <FastSIMD/Utility/FeatureEnums.h>

namespace FastSIMD
{
    template<int, FeatureSet...>
    struct FeatureSetList;

    template<FeatureSet HEAD>
    struct FeatureSetList<0, HEAD>
    {
        static constexpr FeatureSet AsArray[] = { HEAD };
        static constexpr FeatureSet Minimum = HEAD;
        static constexpr FeatureSet Maximum = HEAD;

        template<FeatureSet L>
        static constexpr FeatureSet NextAfter = FeatureSet::Max;
    };

    template<FeatureSet HEAD, FeatureSet... TAIL>
    struct FeatureSetList<0, HEAD, TAIL...>
    {
        static constexpr FeatureSet AsArray[] = { HEAD, TAIL... };
        static constexpr FeatureSet Minimum = HEAD;
        static constexpr FeatureSet Maximum = FeatureSetList<0, TAIL...>::Maximum;

        template<FeatureSet L>
        static constexpr FeatureSet NextAfter = (L == HEAD) ? FeatureSetList<0, TAIL...>::Minimum : FeatureSetList<0, TAIL...>::template NextAfter<L>;
    };

}
