#pragma once

#ifndef FASTSIMD_INCLUDE_EXT
#error Include ext not defined
#endif

#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X

#define CAT(X,Y) CAT2(X,Y)
#define CAT2(X,Y) X##Y

#define FASTSIMD_INCLUDE(_includeFile) STRINGIFY(CAT(_includeFile, FASTSIMD_INCLUDE_EXT))
