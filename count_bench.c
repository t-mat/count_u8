// Benchmark and test program for count_u8.h and count_u16.h
//
//  SPDX-FileCopyrightText: Copyright (c) Takayuki Matsuoka
//  SPDX-License-Identifier: CC0-1.0
//  https://spdx.org/licenses/CC0-1.0
//  https://creativecommons.org/publicdomain/zero/1.0/

#include "count_u8.h"
#include "count_u16.h"
#include <stdio.h>
#include <string.h>

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
    const clock_t t = clock();
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


static void fill_random(uint8_t* mem, size_t memSizeInBytes, uint64_t seed) {
    uint64_t y = seed;
    for(size_t i = 0; i < memSizeInBytes; ++i) {
        y ^= y << 11;   // xorshift PRNG
        y ^= y >> 31;
        y ^= y << 18;
        mem[i] = (uint8_t) y;
    }
}


static void bench_u8(uint8_t* mem, size_t memSizeInBytes) {
    printf("bench_u8()\n");

    fill_random(mem, memSizeInBytes, 0x0123456789abcdefULL);

    enum { nValue = 256 };

    // Scalar (naive)
    static size_t naive_counters[nValue] = { 0 };
    double naive_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            naive_counters[v] = count_u8_scalar_naive(mem, memSizeInBytes, (uint8_t) v);
        }
        naive_duration = end_clock(start);
    }

    // Scalar (intloop)
    static size_t intloop_counters[nValue] = { 0 };
    double intloop_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            intloop_counters[v] = count_u8_scalar_intloop(mem, memSizeInBytes, (uint8_t) v);
        }
        intloop_duration = end_clock(start);
    }

    // Scalar (default)
    static size_t scalar_counters[nValue] = { 0 };
    double scalar_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            scalar_counters[v] = count_u8_scalar(mem, memSizeInBytes, (uint8_t) v);
        }
        scalar_duration = end_clock(start);
    }

    // SSE2
    static size_t sse2_counters[nValue] = { 0 };
    double sse2_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            sse2_counters[v] = count_u8_sse2(mem, memSizeInBytes, (uint8_t) v);
        }
        sse2_duration = end_clock(start);
    }

    // Default
    static size_t default_counters[nValue] = { 0 };
    double default_duration = 0;
    {
        clock_t start = start_clock();
        for(int v = 0; v < nValue; ++v) {
            default_counters[v] = count_u8(mem, memSizeInBytes, (uint8_t) v);
        }
        default_duration = end_clock(start);
    }

    // Verify
    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != scalar_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, scalar=%10zd\n", i, naive_counters[i], scalar_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != intloop_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, intloop=%10zd\n", i, naive_counters[i], intloop_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != sse2_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, sse2=%10zd\n", i, naive_counters[i], sse2_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != default_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, default=%10zd\n", i, naive_counters[i], default_counters[i]);
        }
    }

    // Result
    printf("Naive   in%8.5f sec, speed%8.2f%%\n", naive_duration,   100.0 * scalar_duration / naive_duration);
    printf("IntLoop in%8.5f sec, speed%8.2f%%\n", intloop_duration, 100.0 * scalar_duration / intloop_duration);
    printf("Scalar  in%8.5f sec, speed%8.2f%%\n", scalar_duration,  100.0 * scalar_duration / scalar_duration);
    printf("SSE2    in%8.5f sec, speed%8.2f%%\n", sse2_duration,    100.0 * scalar_duration / sse2_duration);
    printf("Default in%8.5f sec, speed%8.2f%%\n", default_duration, 100.0 * scalar_duration / default_duration);
}


static void fill_random_u16(uint8_t* mem, size_t memSizeInBytes, uint64_t seed, uint64_t mult) {
    uint64_t y = seed;
    for(size_t i = 0; i < memSizeInBytes; i += 4) {
        y ^= y << 11;   // xorshift PRNG
        y ^= y >> 31;
        y ^= y << 18;
        uint16_t* p = (uint16_t*) &mem[i];
        *p = (uint16_t) ((y & 0xff) * mult);
    }
}


static void bench_u16(uint8_t* mem, size_t memSizeInBytes) {
    printf("bench_u16()\n");

    const uint32_t mult = 0x12341357;

    fill_random_u16(mem, memSizeInBytes, 0x0123456789abcdefULL, mult);

    enum { nValue = 256 };

    // Scalar (naive)
    static size_t naive_counters[nValue] = { 0 };
    double naive_duration = 0;
    {
        clock_t start = start_clock();
        for(int value = 0; value < nValue; ++value) {
            const uint32_t vl = value * mult;
            naive_counters[value] = count_u16_scalar_naive(mem, memSizeInBytes, vl);
        }
        naive_duration = end_clock(start);
    }

    // Scalar (intloop)
    static size_t intloop_counters[nValue] = { 0 };
    double intloop_duration = 0;
    {
        clock_t start = start_clock();
        for(int value = 0; value < nValue; ++value) {
            const uint32_t vl = value * mult;
            intloop_counters[value] = count_u16_scalar_intloop(mem, memSizeInBytes, vl);
        }
        intloop_duration = end_clock(start);
    }

    // Scalar (default)
    static size_t scalar_counters[nValue] = { 0 };
    double scalar_duration = 0;
    {
        clock_t start = start_clock();
        for(int value = 0; value < nValue; ++value) {
            const uint32_t vl = value * mult;
            scalar_counters[value] = count_u16_scalar(mem, memSizeInBytes, vl);
        }
        scalar_duration = end_clock(start);
    }

    // SSE2
    static size_t sse2_counters[nValue] = { 0 };
    double sse2_duration = 0;
    {
        clock_t start = start_clock();
        for(int value = 0; value < nValue; ++value) {
            const uint32_t vl = value * mult;
            sse2_counters[value] = count_u16_sse2(mem, memSizeInBytes, vl);
        }
        sse2_duration = end_clock(start);
    }

    // Default
    static size_t default_counters[nValue] = { 0 };
    double default_duration = 0;
    {
        clock_t start = start_clock();
        for(int value = 0; value < nValue; ++value) {
            const uint32_t vl = value * mult;
            default_counters[value] = count_u16(mem, memSizeInBytes, vl);
        }
        default_duration = end_clock(start);
    }

    // Verify
    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != scalar_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, scalar=%10zd\n", i, naive_counters[i], scalar_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != intloop_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, intloop=%10zd\n", i, naive_counters[i], intloop_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != sse2_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, sse2=%10zd\n", i, naive_counters[i], sse2_counters[i]);
        }
    }

    for(int i = 0; i < nValue; ++i) {
        if(naive_counters[i] != default_counters[i]) {
            printf("Error: i=%3d, naive=%10zd, default=%10zd\n", i, naive_counters[i], default_counters[i]);
        }
    }

    // Result
    printf("Naive   in%8.5f sec, speed%8.2f%%\n", naive_duration,   100.0 * scalar_duration / naive_duration);
    printf("IntLoop in%8.5f sec, speed%8.2f%%\n", intloop_duration, 100.0 * scalar_duration / intloop_duration);
    printf("Scalar  in%8.5f sec, speed%8.2f%%\n", scalar_duration,  100.0 * scalar_duration / scalar_duration);
    printf("SSE2    in%8.5f sec, speed%8.2f%%\n", sse2_duration,    100.0 * scalar_duration / sse2_duration);
    printf("Default in%8.5f sec, speed%8.2f%%\n", default_duration, 100.0 * scalar_duration / default_duration);
}


int main(int argc, char** argv) {
    int enable_bench_u8  = 0;
    int enable_bench_u16 = 0;
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--u8")  == 0) { enable_bench_u8  = 1; continue; }
        if(strcmp(argv[i], "--u16") == 0) { enable_bench_u16 = 1; continue; }
    }
    if(enable_bench_u8 == 0 && enable_bench_u16 == 0) {
        enable_bench_u8  = 1;
        enable_bench_u16 = 1;
    }

    // Since MSVC doesn't have C11 standard aligned_alloc(),
    // we use _mm_malloc() and _mm_free().
    // See also: https://docs.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance?view=msvc-160#note_M
    const size_t size = 1024 * 1024 * 32;
    const size_t alignment = 65536;
    void* mem = _mm_malloc(size, alignment);
    if(enable_bench_u8)  { bench_u8((uint8_t*) mem, size);  }
    if(enable_bench_u16) { bench_u16((uint8_t*) mem, size); }
    _mm_free(mem);
    return 0;
}
