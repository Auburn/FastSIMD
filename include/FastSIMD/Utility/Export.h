#pragma once

#if 0// !defined( FASTSIMD_STATIC_LIB ) && ( defined( _WIN32 ) || defined( __CYGWIN__ ) )
#if 1//def FASTSIMD_EXPORT // CHANGE ME
#define FASTSIMD_API __declspec( dllexport )
#else
#define FASTSIMD_API __declspec( dllimport )
#endif
#else
#define FASTSIMD_API
#endif
