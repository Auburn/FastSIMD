#pragma once

#ifndef FASTSIMD_BUILD_CLASS
#error Do not include this file
#endif

#ifdef FASTSIMD_INCLUDE_HEADER_ONLY
#include "../Example/Example.h"
#else
#include "../Example/Example.cpp"
#endif
FASTSIMD_BUILD_CLASS( Example )

