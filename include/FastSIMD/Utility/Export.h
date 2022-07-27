#pragma once

#if !defined( FASTSIMD_STATIC_LIB ) && ( defined( _WIN32 ) || defined( __CYGWIN__ ) )
#ifdef FASTSIMD_EXPORT
#define FASTSIMD_API __declspec( dllexport )
#else
#define FASTSIMD_API __declspec( dllimport )
#endif
#else
#define FASTSIMD_API
#endif
