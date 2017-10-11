#pragma once

#include "FastSIMD.h"
#include "simdtypes/TypeList.h"

#ifndef FS_SIMD_CLASS
#define FS_SIMD_CLASS FastSIMD_Scalar

#define FASTSIMD_BUILD_CLASS(_CLASS) \
template FS_CLASS(_CLASS) <FS_SIMD_CLASS>;
#endif

namespace FastSIMD
{
	template<typename FirstT, typename... Types>
	struct MultiTemplate
	{
		typedef typename MultiTemplate<Types...>::type type;
	};

	template<typename... Types>
	struct MultiTemplate<FS_SIMD_CLASS, Types...>
	{
		typedef typename FS_SIMD_CLASS type;
	};

	template<typename FirstT>
	struct MultiTemplate<FirstT>
	{
		typedef FirstT type;
	};
}

#define FS_MULTI_TEMPLATE(...) FastSIMD::MultiTemplate<__VA_ARGS__>::type
