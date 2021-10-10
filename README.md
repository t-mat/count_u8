# count_u8
Count the number of uint8_t or uint16_t elements in memory region.  Header-only library in C99.  Optimized for SSE2.

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

`count_u8()` and `count_u16()` automatically detect SSE2 by compiler's
predefined symbols such as `__SSE2__`, `_M_X64`.


## Benchmark

### gcc

```
$ CC=gcc-10 make clean all
bench_u8()
Scalar  in 1.19409 sec, speed  100.00%
SSE2    in 0.13273 sec, speed  899.64%
Default in 0.12824 sec, speed  931.10%
bench_u16()
Scalar  in 2.37073 sec, speed  100.00%
SSE2    in 0.32686 sec, speed  725.30%
Default in 0.32399 sec, speed  731.73%
```


### clang

```
$ CC=clang-12 make clean all
bench_u8()
Scalar  in 2.16803 sec, speed  100.00%
SSE2    in 0.13012 sec, speed 1666.12%
Default in 0.12546 sec, speed 1728.04%
bench_u16()
Scalar  in 2.09604 sec, speed  100.00%
SSE2    in 0.29672 sec, speed  706.40%
Default in 0.29899 sec, speed  701.04%
```


## MSVC

```
# MSVC 2019
bench_u8()
Scalar  in 2.21406 sec, speed  100.00%
SSE2    in 0.13927 sec, speed 1589.77%
Default in 0.13581 sec, speed 1630.31%
bench_u16()
Scalar  in 4.30308 sec, speed  100.00%
SSE2    in 0.28564 sec, speed 1506.49%
Default in 0.28825 sec, speed 1492.82%
```
