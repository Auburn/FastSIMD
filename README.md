# FastSIMD

FastSIMD is an SIMD abstraction layer that allows easy development of SIMD code. Using the generic SIMD register types included with FastSIMD provides access to standard C++ operators and various mathematic functions. Using template parameters or a define the generic types can be compiled to various SIMD feature sets (SSE, AVX, NEON...)

FastSIMD also provides an easy to use class template for runtime SIMD feature set detection and appropriate function dispatch. The generic register types mentioned above make it easy to write code that always makes use of the highest SIMD feature set available.

The original version of FastSIMD was developed along side FastNoise2. This new "1.0" version of FastSIMD is being developed as a working standalone library, although the development is still primarily driven to support FastNoise2. Compared to the "old" version of FastSIMD supplied with FastNoise2 this "1.0" version of FastSIMD has these improvements:

- Removed all uses of C++ macros in favour of templated types/functions 
- Variable sized generic register types. For example when using operators on a register of 8xInt32 when targeting SSE, the intrinsics output will get doubled up transparently.
- Moved from SIMD levels to FeatureFlags to allow more verbose specialisation of templated types and more readable code.
