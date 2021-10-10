# count_u8
Count the number of uint8_t, uint16_t or uint32_t elements in memory region.  Header-only library in C99.  Optimized for SSE2.

## Usage

```c
#include "count_u8.h"

void test() {
    size_t bufSize = 65536;
    uint8_t* buf = (uint8_t*) malloc(bufSize);

    ... set_some_values(buf, bufSize); ...

    uint8_t value = 0x42;
    size_t numElem = count_u8(buf, bufSize, value);
    printf("numElem for %02x = %zd\n", value, numElem);
}
```

```c
#include "count_u16.h"

void test() {
    size_t bufSize = 65536;
    uint8_t* buf = (uint8_t*) malloc(bufSize);

    ... set_some_values(buf, bufSize); ...

    uint16_t value = 0x4251;
    size_t numElem = count_u16(buf, bufSize, value);
    printf("numElem for %04x = %zd\n", value, numElem);
}
```

```c
#include "count_u32.h"

void test() {
    size_t bufSize = 65536;
    uint8_t* buf = (uint8_t*) malloc(bufSize);

    ... set_some_values(buf, bufSize); ...

    uint132_t value = 0xcafe4251;
    size_t numElem = count_u32(buf, bufSize, value);
    printf("numElem for %04x = %zd\n", value, numElem);
}
```

`count_u8()`, `count_u16()` and `count_u32()` automatically detect SSE2 by compiler's
predefined symbols such as `__SSE2__`, `_M_X64`.


## Benchmark

### gcc

```
$ CC=gcc-10 make clean all
bench_u8()
Scalar  in 2.33275 sec, speed  100.00%
SSE2    in 0.36099 sec, speed  646.20%
Default in 0.33875 sec, speed  688.64%
bench_u16()
Scalar  in 1.18196 sec, speed  100.00%
SSE2    in 0.35938 sec, speed  328.89%
Default in 0.33277 sec, speed  355.19%
bench_u32()
Scalar  in 0.44017 sec, speed  100.00%
SSE2    in 0.33748 sec, speed  130.43%
Default in 0.32490 sec, speed  135.48%
```


### clang

```
$ CC=clang-12 make clean all
bench_u8()
Scalar  in 4.25728 sec, speed  100.00%
SSE2    in 0.33818 sec, speed 1258.87%
Default in 0.33461 sec, speed 1272.30%
bench_u16()
Scalar  in 1.23361 sec, speed  100.00%
SSE2    in 0.33544 sec, speed  367.75%
Default in 0.34250 sec, speed  360.18%
bench_u32()
Scalar  in 0.58973 sec, speed  100.00%
SSE2    in 0.33374 sec, speed  176.70%
Default in 0.33105 sec, speed  178.14%
```


## MSVC

```
# MSVC 2019
bench_u8()
Scalar  in 4.38357 sec, speed  100.00%
SSE2    in 0.36108 sec, speed 1214.00%
Default in 0.34013 sec, speed 1288.78%
bench_u16()
Scalar  in 2.28388 sec, speed  100.00%
SSE2    in 0.34870 sec, speed  654.97%
Default in 0.34179 sec, speed  668.21%
bench_u32()
Scalar  in 1.22360 sec, speed  100.00%
SSE2    in 0.34426 sec, speed  355.43%
Default in 0.32954 sec, speed  371.31%
```
