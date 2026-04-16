// Count the number of uint16_t elements in memory region.
// Header-only library in C99.  Optimized for SSE2 / AVX2.
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
//  predefined symbols such as __AVX2__, __SSE2__, _M_X64.
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

// Scalar (naive)
static inline size_t count_u16_scalar_naive(const void* src, size_t srcSizeInBytes, uint16_t value) {
    size_t counter = 0;
    const uint16_t* const data = (const uint16_t*) src;
    for(size_t i = 0; i < srcSizeInBytes/sizeof(*data); ++i) {
        if(data[i] == value) {
            counter += 1;
        }
    }
    return counter;
}


// Scalar (int loop counter)
static inline size_t count_u16_scalar_intloop(const void* src, size_t srcSizeInBytes, uint16_t value) {
    uint64_t counter = 0;

    const uint16_t* data = (const uint16_t*) src;

    for(uint64_t nd = (uint64_t) srcSizeInBytes/sizeof(*data); nd > 0; ) {
        int32_t n;

        const uint64_t nMax = INT32_MAX;
        if(nd < nMax) {
            n = (int) nd;
        } else {
            n = (int) nMax;
        }
        nd -= n;

        // "int32_t" for finding and loop counter helps compiler's optimization (gcc, clang).
        int32_t cnt = 0;
        for(int32_t i = 0; i < n; ++i) {
            if(data[i] == value) {
                cnt += 1;
            }
        }
        data += n;
        counter += (uint64_t) cnt;
    }

    return (size_t) counter;
}


// Scalar
static inline size_t count_u16_scalar(const void* src, size_t srcSizeInBytes, uint16_t value) {
#if _MSC_VER
    return count_u16_scalar_naive(src, srcSizeInBytes, value);
#else
    return count_u16_scalar_intloop(src, srcSizeInBytes, value);
#endif
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

#if 0
        __m128i sumt_32x4;
        sumt_32x4 = _mm_add_epi32(sum0_32x4, sum1_32x4);
        sumt_32x4 = _mm_add_epi32(sumt_32x4, sum2_32x4);
        sumt_32x4 = _mm_add_epi32(sumt_32x4, sum3_32x4);

        uint32_t counters[4];
        _mm_storeu_si128((__m128i*) counters, sumt_32x4);
        simdPartCounter  = (uint64_t) counters[0] + (uint64_t) counters[1];
        simdPartCounter += (uint64_t) counters[2] + (uint64_t) counters[3];
#else
        uint32_t counters[4][4];
        _mm_storeu_si128((__m128i*) &counters[0], sum0_32x4);
        _mm_storeu_si128((__m128i*) &counters[1], sum1_32x4);
        _mm_storeu_si128((__m128i*) &counters[2], sum2_32x4);
        _mm_storeu_si128((__m128i*) &counters[3], sum3_32x4);
        for(int i = 0; i < 4; ++i) {
            simdPartCounter += (uint64_t) (counters[i][0]) + (uint64_t) (counters[i][1]);
            simdPartCounter += (uint64_t) (counters[i][2]) + (uint64_t) (counters[i][3]);
        }
#endif
    }

    uint64_t lastPartCounter = 0;
    for(const uint16_t* q = (const uint16_t*) endOfSimdPart; q < (const uint16_t*) endOfData; ++q) {
        lastPartCounter += (*q == value) ? 1 : 0;
    }

    return (size_t) (simdPartCounter + lastPartCounter);
}


// AVX2
//
//  Same structure as count_u16_sse2 (cmpeq_epi16 -> periodically folded via
//  madd_epi16 before int16 saturation), widened to 256-bit.  A 16-bit lane
//  can hit its int16 minimum after 32768 increments, so we still fold every
//  `maxBytes = 32768 * bytesPerLoop` bytes.
#if defined(__AVX2__)
static inline size_t count_u16_avx2(const void* src, size_t srcSizeInBytes, uint16_t value) {
    const uint64_t          bytesPerLoop    = 32 * 4;
    const int               prefetchLen     = 4096;

    const uint64_t          srcSize         = srcSizeInBytes & (~1);
    const uint8_t* const    data            = (const uint8_t*) src;
    const uint8_t* const    endOfData       = data + srcSize;
    const uint8_t* const    endOfSimdPart   = endOfData - (srcSize % bytesPerLoop);

    uint64_t simdPartCounter = 0;
    {
        __m256i         sum0_32x8   = _mm256_setzero_si256();
        __m256i         sum1_32x8   = _mm256_setzero_si256();
        __m256i         sum2_32x8   = _mm256_setzero_si256();
        __m256i         sum3_32x8   = _mm256_setzero_si256();
        const __m256i   c_16x16     = _mm256_set1_epi16((int16_t) value);

        for(const uint8_t* p = data; p < endOfSimdPart; ) {
            int64_t restInBytes = (endOfSimdPart - p);

            const int64_t maxBytes = 32768 * bytesPerLoop;
            if(restInBytes > maxBytes) {
                restInBytes = maxBytes;
            }

            const uint8_t* elp = p + restInBytes;

            __m256i     sum0_16x16  = _mm256_setzero_si256();
            __m256i     sum1_16x16  = _mm256_setzero_si256();
            __m256i     sum2_16x16  = _mm256_setzero_si256();
            __m256i     sum3_16x16  = _mm256_setzero_si256();

            for(; p < elp; p += bytesPerLoop) {
                const __m256i*  m               = (const __m256i *) p;
                const __m256i   cmp0_16x16      = _mm256_cmpeq_epi16(c_16x16, _mm256_loadu_si256(m  ));
                const __m256i   cmp1_16x16      = _mm256_cmpeq_epi16(c_16x16, _mm256_loadu_si256(m+1));
                const __m256i   cmp2_16x16      = _mm256_cmpeq_epi16(c_16x16, _mm256_loadu_si256(m+2));
                const __m256i   cmp3_16x16      = _mm256_cmpeq_epi16(c_16x16, _mm256_loadu_si256(m+3));

                const uint8_t*  prefetchPtr     = p + prefetchLen;
    #if defined(_MSC_VER)
                _mm_prefetch((const char*) prefetchPtr, _MM_HINT_T0);
    #elif defined(__GNUC__)
                __builtin_prefetch(prefetchPtr, 0, 3);
    #else
    #  error
    #endif
                sum0_16x16 = _mm256_add_epi16(sum0_16x16, cmp0_16x16);
                sum1_16x16 = _mm256_add_epi16(sum1_16x16, cmp1_16x16);
                sum2_16x16 = _mm256_add_epi16(sum2_16x16, cmp2_16x16);
                sum3_16x16 = _mm256_add_epi16(sum3_16x16, cmp3_16x16);
            }

            const __m256i   k_16x16         = _mm256_set1_epi16((int16_t) -1);
            const __m256i   horsum0_32x8    = _mm256_madd_epi16(sum0_16x16, k_16x16);
            const __m256i   horsum1_32x8    = _mm256_madd_epi16(sum1_16x16, k_16x16);
            const __m256i   horsum2_32x8    = _mm256_madd_epi16(sum2_16x16, k_16x16);
            const __m256i   horsum3_32x8    = _mm256_madd_epi16(sum3_16x16, k_16x16);

            sum0_32x8 = _mm256_add_epi32(sum0_32x8, horsum0_32x8);
            sum1_32x8 = _mm256_add_epi32(sum1_32x8, horsum1_32x8);
            sum2_32x8 = _mm256_add_epi32(sum2_32x8, horsum2_32x8);
            sum3_32x8 = _mm256_add_epi32(sum3_32x8, horsum3_32x8);
        }

        uint32_t counters[4][8];
        _mm256_storeu_si256((__m256i*) &counters[0], sum0_32x8);
        _mm256_storeu_si256((__m256i*) &counters[1], sum1_32x8);
        _mm256_storeu_si256((__m256i*) &counters[2], sum2_32x8);
        _mm256_storeu_si256((__m256i*) &counters[3], sum3_32x8);
        for(int i = 0; i < 4; ++i) {
            for(int j = 0; j < 8; ++j) {
                simdPartCounter += (uint64_t) (counters[i][j]);
            }
        }
    }

    uint64_t lastPartCounter = 0;
    for(const uint16_t* q = (const uint16_t*) endOfSimdPart; q < (const uint16_t*) endOfData; ++q) {
        lastPartCounter += (*q == value) ? 1 : 0;
    }

    return (size_t) (simdPartCounter + lastPartCounter);
}
#endif // __AVX2__


// "Default".  Select AVX2 / SSE2 if it's available.
static inline size_t count_u16(const void* src, size_t srcSize, uint16_t value) {
#if defined(__AVX2__)
    return count_u16_avx2(src, srcSize, value);
#elif defined(__SSE2__)   /* generic */ \
   || defined(__x86_64__) /* gcc */ \
   || defined(_M_X64)     /* MSVC, https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros */
    return count_u16_sse2(src, srcSize, value);
#else
    return count_u16_scalar(src, srcSize, value);
#endif
}

#endif // COUNT_U16_H
