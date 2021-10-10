// Count the number of uint16_t elements in memory region.
// Header-only library in C99.  Optimized for SSE2.
//
// # Usage
//
//      size_t bufSize = 65536;
//      uint8_t* buf = (uint8_t*) malloc(bufSize);
//
//      ... set_some_values(buf, bufSize); ...
//
//      uint16_t value = 0x4251;
//      size_t numElem = count_u16(buf, bufSize, value);
//
//  count_u16() automatically detect supported instruction by compiler's
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

#ifndef COUNT_U16_H
#define COUNT_U16_H

#include <stdint.h>

#if defined(_MSC_VER)
#  include <intrin.h>
#elif defined(__GNUC__)
#  include <x86intrin.h>
#else
#  error
#endif

// Scalar
static inline size_t count_u16_scalar(const void* src, size_t srcSizeInBytes, uint16_t value) {
    size_t counter = 0;
    const uint16_t* const data = (const uint16_t*) src;
    for(size_t i = 0; i < srcSizeInBytes/2; ++i) {
        if(data[i] == value) {
            counter += 1;
        }
    }
    return counter;
}


// SSE2
static inline size_t count_u16_sse2(const void* src, size_t srcSizeInBytes, uint16_t value) {
    const uint64_t          bytesPerLoop    = 16 * 4;
    const int               prefetchLen     = 4096;

    const uint64_t          srcSize         = srcSizeInBytes & (~1);
    const uint8_t* const    data            = (const uint8_t*) src;
    const uint8_t* const    endOfData       = data + srcSize;
    const uint8_t* const    endOfSimdPart   = endOfData - (srcSize % bytesPerLoop);

    uint64_t simdPartCounter = 0;
    {
        __m128i         sum0_32x4   = _mm_setzero_si128();
        __m128i         sum1_32x4   = _mm_setzero_si128();
        __m128i         sum2_32x4   = _mm_setzero_si128();
        __m128i         sum3_32x4   = _mm_setzero_si128();
        const __m128i   c_16x8      = _mm_set1_epi16(value);

        for(const uint8_t* p = data; p < endOfSimdPart; ) {
            int64_t restInBytes = (endOfSimdPart - p);

            const int64_t maxBytes = 32768 * bytesPerLoop;
            if(restInBytes > maxBytes) {
                restInBytes = maxBytes;
            }

            const uint8_t* elp = p + restInBytes;

            __m128i     sum0_16x8   = _mm_setzero_si128();
            __m128i     sum1_16x8   = _mm_setzero_si128();
            __m128i     sum2_16x8   = _mm_setzero_si128();
            __m128i     sum3_16x8   = _mm_setzero_si128();

            for(; p < elp; p += bytesPerLoop) {
                const __m128i*  m               = (const __m128i *) p;
                const __m128i   cmp0_16x8       = _mm_cmpeq_epi16(c_16x8, _mm_loadu_si128(m  ));
                const __m128i   cmp1_16x8       = _mm_cmpeq_epi16(c_16x8, _mm_loadu_si128(m+1));
                const __m128i   cmp2_16x8       = _mm_cmpeq_epi16(c_16x8, _mm_loadu_si128(m+2));
                const __m128i   cmp3_16x8       = _mm_cmpeq_epi16(c_16x8, _mm_loadu_si128(m+3));

                const uint8_t*  prefetchPtr     = p + prefetchLen;
    #if defined(_MSC_VER)
                _mm_prefetch((const char*) prefetchPtr, _MM_HINT_T0);
    #elif defined(__GNUC__)
                __builtin_prefetch(prefetchPtr, 0, 3);
    #else
    #  error
    #endif
                sum0_16x8 = _mm_add_epi16(sum0_16x8, cmp0_16x8);
                sum1_16x8 = _mm_add_epi16(sum1_16x8, cmp1_16x8);
                sum2_16x8 = _mm_add_epi16(sum2_16x8, cmp2_16x8);
                sum3_16x8 = _mm_add_epi16(sum3_16x8, cmp3_16x8);
            }

            const __m128i   k_16x8          = _mm_set1_epi16((int16_t) -1);
            const __m128i   horsum0_32x4    = _mm_madd_epi16(sum0_16x8, k_16x8);
            const __m128i   horsum1_32x4    = _mm_madd_epi16(sum1_16x8, k_16x8);
            const __m128i   horsum2_32x4    = _mm_madd_epi16(sum2_16x8, k_16x8);
            const __m128i   horsum3_32x4    = _mm_madd_epi16(sum3_16x8, k_16x8);

            sum0_32x4 = _mm_add_epi32(sum0_32x4, horsum0_32x4);
            sum1_32x4 = _mm_add_epi32(sum1_32x4, horsum1_32x4);
            sum2_32x4 = _mm_add_epi32(sum2_32x4, horsum2_32x4);
            sum3_32x4 = _mm_add_epi32(sum3_32x4, horsum3_32x4);
        }

        __m128i sumt_32x4;
        sumt_32x4 = _mm_add_epi32(sum0_32x4, sum1_32x4);
        sumt_32x4 = _mm_add_epi32(sumt_32x4, sum2_32x4);
        sumt_32x4 = _mm_add_epi32(sumt_32x4, sum3_32x4);

        uint32_t counters[4];
        _mm_storeu_si128((__m128i*) counters, sumt_32x4);

        simdPartCounter  = (uint64_t) counters[0] + (uint64_t) counters[1];
        simdPartCounter += (uint64_t) counters[2] + (uint64_t) counters[3];
    }

    uint64_t lastPartCounter = 0;
    for(const uint16_t* q = (const uint16_t*) endOfSimdPart; q < (const uint16_t*) endOfData; ++q) {
        lastPartCounter += (*q == value) ? 1 : 0;
    }

    return (size_t) (simdPartCounter + lastPartCounter);
}


// "Default".  Select SSE2 if it's available.
static inline size_t count_u16(const void* src, size_t srcSize, uint16_t value) {
#if defined(__SSE2__)   /* generic */ \
 || defined(__x86_64__) /* gcc */ \
 || defined(_M_X64)     /* MSVC, https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros */
    return count_u16_sse2(src, srcSize, value);
#else
    return count_u16_scalar(src, srcSize, value);
#endif
}

#endif // COUNT_U16_H
