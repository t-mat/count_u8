// Benchmark for count_u8.h
//
//  SPDX-FileCopyrightText: Copyright (c) Takayuki Matsuoka
//  SPDX-License-Identifier: CC0-1.0
//  https://spdx.org/licenses/CC0-1.0
//  https://creativecommons.org/publicdomain/zero/1.0/

#include "count_u8.h"
#include <stdio.h>

#if _MSC_VER
#  include <windows.h>
typedef uint64_t clock_t;
#else
#  include <time.h>       // clock(), clock_t
#endif

static clock_t start_clock(void) {
#if _MSC_VER
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
#else
    clock_t t = clock();
    for(;;) {
        clock_t s = clock();
        if(s != t) {
            return s;
        }
    }
#endif
}

static double end_clock(clock_t start) {
#if _MSC_VER
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    return (double) (end.QuadPart - start) / (double) freq.QuadPart;
#else
    return (clock() - start) / ((double) CLOCKS_PER_SEC);
#endif
}

static void fillRandom(uint8_t* mem, size_t size, uint64_t seed) {
    uint64_t y = seed;
    for(size_t i = 0; i < size; ++i) {
        y ^= y << 11;   // xorshift PRNG
        y ^= y >> 31;
        y ^= y << 18;
        mem[i] = (uint8_t) y;
    }
}

static void bench(uint8_t* mem, size_t size) {
    fillRandom(mem, size, 0x0123456789abcdefULL);

    enum { nValue = 256 };

    // Scalar
    static size_t scalar_counters[nValue] = { 0 };
    double scalar_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            scalar_counters[v] = count_u8_scalar(mem, size, (uint8_t) v);
        }
        scalar_duration = end_clock(start);
    }

    // SSE2
    static size_t sse2_counters[nValue] = { 0 };
    double sse2_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            sse2_counters[v] = count_u8_sse2(mem, size, (uint8_t) v);
        }
        sse2_duration = end_clock(start);
    }

    // Default
    static size_t default_counters[nValue] = { 0 };
    double default_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            default_counters[v] = count_u8(mem, size, (uint8_t) v);
        }
        default_duration = end_clock(start);
    }

    // Verify
    for(int i = 0; i < nValue; ++i) {
        if(scalar_counters[i] != sse2_counters[i]) {
            printf("Error: i=%3d, scalar=%10zd, sse2=%10zd\n", i, scalar_counters[i], sse2_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(scalar_counters[i] != default_counters[i]) {
            printf("Error: i=%3d, scalar=%10zd, default=%10zd\n", i, scalar_counters[i], default_counters[i]);
        }
    }

    // Result
    printf("Scalar  in%8.5f sec, speed%8.2f%%\n", scalar_duration, 100.0);
    printf("SSE2    in%8.5f sec, speed%8.2f%%\n", sse2_duration, 100.0 * scalar_duration / sse2_duration);
    printf("Default in%8.5f sec, speed%8.2f%%\n", default_duration, 100.0 * scalar_duration / default_duration);
}

int main(int argc, char** argv) {
    // Since MSVC doesn't have C11 standard aligned_alloc(),
    // we use _mm_malloc() and _mm_free().
    // See also: https://docs.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance?view=msvc-160#note_M
    const size_t size = 1024 * 1024 * 16;
    const size_t alignment = 65536;
    void* mem = _mm_malloc(size, alignment);
    bench((uint8_t*) mem, size);
    _mm_free(mem);
    return 0;
}
