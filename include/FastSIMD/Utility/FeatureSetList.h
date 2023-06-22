#pragma once
#include <FastSIMD/Utility/FeatureEnums.h>

namespace FastSIMD
{
    template<FeatureSet...>
    struct FeatureSetList;

    template<FeatureSet HEAD>
    struct FeatureSetList<HEAD>
    {
        static constexpr FeatureSet Minimum = HEAD;
        static constexpr FeatureSet Maximum = HEAD;

        template<FeatureSet L>
        static constexpr FeatureSet NextAfter = FeatureSet::Max;
    };

    template<FeatureSet HEAD, FeatureSet... TAIL>
    struct FeatureSetList<HEAD, TAIL...>
    {
        static constexpr FeatureSet Minimum = HEAD;
        static constexpr FeatureSet Maximum = FeatureSetList<TAIL...>::Maximum;

        template<FeatureSet L>
        static constexpr FeatureSet NextAfter = (L == HEAD) ? FeatureSetList<TAIL...>::Minimum : FeatureSetList<TAIL...>::template NextAfter<L>;
    };

}
