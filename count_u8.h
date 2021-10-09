// Count the number of uint8_t elements in memory region.
// Header-only library in C99.  Optimized for SSE2.
//
// # Usage
//
//      size_t bufSize = 65536;
//      uint8_t* buf = (uint8_t*) malloc(bufSize);
//
//      ... set_some_values(buf, bufSize); ...
//
//      uint8_t value = 0x42;
//      size_t numElem = count_u8(buf, bufSize, value);
//
//  count_u8() automatically detect supported instruction by compiler's
//  predefined symbols such as __SSE2__, _M_X64.
//
//
// # Benchmark
//
//      scalar in 2.15700 sec, speed  100.00%
//      SSE2   in 0.13500 sec, speed 1597.78%
//
//
// # References
//
// - Intel Intrinsics Guide
//   https://software.intel.com/sites/landingpage/IntrinsicsGuide/
//
//
// # License
//
//  SPDX-FileCopyrightText: Copyright (c) Takayuki Matsuoka
//  SPDX-License-Identifier: CC0-1.0
//  https://spdx.org/licenses/CC0-1.0
//  https://creativecommons.org/publicdomain/zero/1.0/

#ifndef COUNT_U8_H
#define COUNT_U8_H

#include <stdint.h>

#if defined(_MSC_VER)
#  include <intrin.h>
#elif defined(__GNUC__)
#  include <x86intrin.h>
#else
#  error
#endif

// Scalar
static inline size_t count_u8_scalar(const void* src, size_t srcSize, uint8_t value) {
    size_t counter = 0;
    const uint8_t* const data = (const uint8_t*) src;
    for(size_t i = 0; i < srcSize; ++i) {
        if(data[i] == value) {
            counter += 1;
        }
    }
    return counter;
}


// SSE2
//
//  note: simdPartOffset
//
//  Suppose we have the following code:
//
//      int ofs = 0x7f;
//      __m128i c_8x16 = _mm_set1_epi8((char) value);
//      __m128i cmp_8x16 = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m));
//      __m128i horsum_64x2 = _mm_sad_epu8(cmp_8x16, _mm_set1_epi8(ofs));
//      sum_64x2 = _mm_add_epi64(sum_64x2, horsum_64x2);
//
//  When all 8bit lanes of cmp_8x16 are 0xff, horsum_64x2 = { 0x400, 0x400 }.
//  When all 8bit lanes of cmp_8x16 are 0x00, horsum_64x2 = { 0x3f8, 0x3f8 }.
//  Here, 0x3f8 == 0x7f * 8 == ofs * 8.
//
//  Therefore, we can compute actual match count by subtracting the
//  "offset" { 0x3f8, 0x3f8 } before add_epi64().  (0x400 - 0x3f8 == 8)
//
//      int ofs = 0x7f;
//      __m128i c_8x16 = _mm_set1_epi8((char) value);
//      __m128i cmp_8x16 = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m));
//      __m128i horsum_64x2 = _mm_sad_epu8(cmp_8x16, _mm_set1_epi8(ofs));
//      horsum_64x2 = _mm_sub_epi64 (horsum_64x2, _mm_set_epi64x(ofs*8, ofs*8)); // offset subtraction
//      sum_64x2 = _mm_add_epi64(sum_64x2, horsum_64x2);
//
//  To remove this offset subtraction from the inner loop, we can compute
//  "total offset" (simdPartOffset).
//
//      simdPartOffset = ofs * bytesPerLoop * numLoop
//
//  And do one subtraction at the outside of the loop.
static inline size_t count_u8_sse2(const void* src, size_t srcSize, uint8_t value) {
    const uint64_t          bytesPerLoop    = 16 * 4;
    const int               prefetchLen     = 4096;

    const uint8_t* const    data            = (const uint8_t*) src;
    const uint8_t* const    endOfData       = data + srcSize;
    const uint8_t* const    endOfSimdPart   = endOfData - (srcSize % bytesPerLoop);
    const uint64_t          numLoop         = (endOfSimdPart - data) / bytesPerLoop;
    const uint64_t          ofs             = 0x7f;
    const uint64_t          simdPartOffset  = ofs * bytesPerLoop * numLoop;

    uint64_t simdPartCounter = 0;
    {
        __m128i         sum0_64x2   = _mm_setzero_si128();
        __m128i         sum1_64x2   = _mm_setzero_si128();
        __m128i         sum2_64x2   = _mm_setzero_si128();
        __m128i         sum3_64x2   = _mm_setzero_si128();
        const __m128i   c_8x16      = _mm_set1_epi8((char) value);
        const __m128i   ofs_8x16    = _mm_set1_epi8((char) ofs);

        for(const uint8_t* p = data; p != endOfSimdPart; p += bytesPerLoop) {
            const __m128i*  m               = (const __m128i *) p;
            const __m128i   cmp0_8x16       = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m  ));
            const __m128i   cmp1_8x16       = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m+1));
            const __m128i   cmp2_8x16       = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m+2));
            const __m128i   cmp3_8x16       = _mm_cmpeq_epi8(c_8x16, _mm_loadu_si128(m+3));

            const uint8_t*  prefetchPtr     = p + prefetchLen;
#if defined(_MSC_VER)
            _mm_prefetch((const char*) prefetchPtr, _MM_HINT_T0);
#elif defined(__GNUC__)
            __builtin_prefetch(prefetchPtr, 0, 3);
#else
#  error
#endif

            const __m128i   horsum0_64x2    = _mm_sad_epu8(cmp0_8x16, ofs_8x16);
            const __m128i   horsum1_64x2    = _mm_sad_epu8(cmp1_8x16, ofs_8x16);
            const __m128i   horsum2_64x2    = _mm_sad_epu8(cmp2_8x16, ofs_8x16);
            const __m128i   horsum3_64x2    = _mm_sad_epu8(cmp3_8x16, ofs_8x16);

            sum0_64x2 = _mm_add_epi64(sum0_64x2, horsum0_64x2);
            sum1_64x2 = _mm_add_epi64(sum1_64x2, horsum1_64x2);
            sum2_64x2 = _mm_add_epi64(sum2_64x2, horsum2_64x2);
            sum3_64x2 = _mm_add_epi64(sum3_64x2, horsum3_64x2);
        }

        uint64_t counters0[2]; _mm_storeu_si128((__m128i*) counters0, sum0_64x2);
        uint64_t counters1[2]; _mm_storeu_si128((__m128i*) counters1, sum1_64x2);
        uint64_t counters2[2]; _mm_storeu_si128((__m128i*) counters2, sum2_64x2);
        uint64_t counters3[2]; _mm_storeu_si128((__m128i*) counters3, sum3_64x2);

        simdPartCounter  = (counters0[0] + counters0[1]);
        simdPartCounter += (counters1[0] + counters1[1]);
        simdPartCounter += (counters2[0] + counters2[1]);
        simdPartCounter += (counters3[0] + counters3[1]);
    }

    uint64_t lastPartCounter = 0;
    for(const uint8_t* q = endOfSimdPart; q != endOfData; ++q) {
        lastPartCounter += (*q == value) ? 1 : 0;
    }

    return (size_t) (simdPartCounter - simdPartOffset + lastPartCounter);
}


// "Default".  Select SSE2 if it's available.
static inline size_t count_u8(const void* src, size_t srcSize, uint8_t value) {
#if defined(__SSE2__)   /* generic */ \
 || defined(__x86_64__) /* gcc */ \
 || defined(_M_X64)     /* MSVC, https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros */
    return count_u8_sse2(src, srcSize, value);
#else
    return count_u8_scalar(src, srcSize, value);
#endif
}

#endif // COUNT_U8_H
