// Count the number of uint32_t elements in memory region.
// Header-only library in C99.  Optimized for SSE2.
//
// # Usage
//
//      size_t bufSize = 65536;
//      uint8_t* buf = (uint8_t*) malloc(bufSize);
//
//      ... set_some_values(buf, bufSize); ...
//
//      uint32_t value = 0xcafe4251;
//      size_t numElem = count_u32(buf, bufSize, value);
//
//  count_u32() automatically detect supported instruction by compiler's
//  predefined symbols such as __SSE2__, _M_X64.
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

#ifndef COUNT_U32_H
#define COUNT_U32_H

#include <stdint.h>

#if defined(_MSC_VER)
#  include <intrin.h>
#elif defined(__GNUC__)
#  include <x86intrin.h>
#else
#  error
#endif

// Scalar
static inline size_t count_u32_scalar(const void* src, size_t srcSize, uint32_t value) {
    size_t counter = 0;
    const uint32_t* const data = (const uint32_t*) src;
    for(size_t i = 0; i < srcSize/4; ++i) {
        if(data[i] == value) {
            counter += 1;
        }
    }
    return counter;
}


// SSE2
static inline size_t count_u32_sse2(const void* src, size_t srcSizeInBytes, uint32_t value) {
    const uint64_t          bytesPerLoop    = 16 * 6;
    const int               prefetchLen     = 4096;

    const uint64_t          srcSize         = srcSizeInBytes & (~3);

    const uint8_t* const    data            = (const uint8_t*) src;
    const uint8_t* const    endOfData       = data + srcSize;
    const uint8_t* const    endOfSimdPart   = endOfData - (srcSize % bytesPerLoop);

    uint64_t simdPartCounter = 0;
    {
        __m128i         sum0_32x4   = _mm_setzero_si128();
        __m128i         sum1_32x4   = _mm_setzero_si128();
        __m128i         sum2_32x4   = _mm_setzero_si128();
        __m128i         sum3_32x4   = _mm_setzero_si128();
        __m128i         sum4_32x4   = _mm_setzero_si128();
        __m128i         sum5_32x4   = _mm_setzero_si128();
        const __m128i   c_32x4      = _mm_set1_epi32(value);

        for(const uint8_t* p = data; p < endOfSimdPart; p += bytesPerLoop) {
            const __m128i*  m               = (const __m128i *) p;
            const __m128i   cmp0_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m  ));
            const __m128i   cmp1_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m+1));
            const __m128i   cmp2_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m+2));
            const __m128i   cmp3_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m+3));
            const __m128i   cmp4_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m+4));
            const __m128i   cmp5_32x4       = _mm_cmpeq_epi32(c_32x4, _mm_loadu_si128(m+5));

            const uint8_t*  prefetchPtr     = p + prefetchLen;
#if defined(_MSC_VER)
            _mm_prefetch((const char*) prefetchPtr, _MM_HINT_T0);
#elif defined(__GNUC__)
            __builtin_prefetch(prefetchPtr, 0, 3);
#else
#  error
#endif
            sum0_32x4 = _mm_add_epi32(sum0_32x4, cmp0_32x4);
            sum1_32x4 = _mm_add_epi32(sum1_32x4, cmp1_32x4);
            sum2_32x4 = _mm_add_epi32(sum2_32x4, cmp2_32x4);
            sum3_32x4 = _mm_add_epi32(sum3_32x4, cmp3_32x4);
            sum4_32x4 = _mm_add_epi32(sum4_32x4, cmp4_32x4);
            sum5_32x4 = _mm_add_epi32(sum5_32x4, cmp5_32x4);
        }

        int32_t counters[6][4];
        _mm_storeu_si128((__m128i*) &counters[0], sum0_32x4);
        _mm_storeu_si128((__m128i*) &counters[1], sum1_32x4);
        _mm_storeu_si128((__m128i*) &counters[2], sum2_32x4);
        _mm_storeu_si128((__m128i*) &counters[3], sum3_32x4);
        _mm_storeu_si128((__m128i*) &counters[4], sum4_32x4);
        _mm_storeu_si128((__m128i*) &counters[5], sum5_32x4);

        for(int i = 0; i < 6; ++i) {
            simdPartCounter += (uint64_t) (-counters[i][0]) + (uint64_t) (-counters[i][1]);
            simdPartCounter += (uint64_t) (-counters[i][2]) + (uint64_t) (-counters[i][3]);
        }
    }

    uint64_t lastPartCounter = 0;
    for(const uint32_t* q = (const uint32_t*) endOfSimdPart; q < (const uint32_t*) endOfData; ++q) {
        lastPartCounter += (*q == value) ? 1 : 0;
    }

    return (size_t) (simdPartCounter + lastPartCounter);
}


// "Default".  Select SSE2 if it's available.
static inline size_t count_u32(const void* src, size_t srcSize, uint32_t value) {
#if defined(__SSE2__)   /* generic */ \
 || defined(__x86_64__) /* gcc */ \
 || defined(_M_X64)     /* MSVC, https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros */
    return count_u32_sse2(src, srcSize, value);
#else
    return count_u32_scalar(src, srcSize, value);
#endif
}

#endif // COUNT_U32_H
