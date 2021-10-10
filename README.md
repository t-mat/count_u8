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
$ CC=gcc-10 make clean bench
bench_u8()
Naive   in 2.36266 sec, speed   55.69%
IntLoop in 1.32088 sec, speed   99.61%
Scalar  in 1.31576 sec, speed  100.00%
SSE2    in 0.39705 sec, speed  331.39%
Default in 0.39365 sec, speed  334.24%
bench_u16()
Naive   in 1.19089 sec, speed   58.18%
IntLoop in 0.69500 sec, speed   99.69%
Scalar  in 0.69283 sec, speed  100.00%
SSE2    in 0.39412 sec, speed  175.79%
Default in 0.38434 sec, speed  180.26%
```


### clang

```
$ CC=clang-12 make clean bench
bench_u8()
Naive   in 4.35857 sec, speed   28.84%
IntLoop in 1.26568 sec, speed   99.30%
Scalar  in 1.25681 sec, speed  100.00%
SSE2    in 0.38171 sec, speed  329.26%
Default in 0.38513 sec, speed  326.33%
bench_u16()
Naive   in 1.25102 sec, speed   49.26%
IntLoop in 0.61198 sec, speed  100.71%
Scalar  in 0.61631 sec, speed  100.00%
SSE2    in 0.37887 sec, speed  162.67%
Default in 0.37887 sec, speed  162.67%
```


## MSVC

```
# MSVC 2019
bench_u8()
Naive   in 2.63528 sec, speed   96.83%
IntLoop in 4.36886 sec, speed   58.41%
Scalar  in 2.55182 sec, speed  100.00%
SSE2    in 0.38719 sec, speed  659.06%
Default in 0.39026 sec, speed  653.89%
bench_u16()
Naive   in 2.31000 sec, speed   98.76%
IntLoop in 2.32072 sec, speed   98.31%
Scalar  in 2.28139 sec, speed  100.00%
SSE2    in 0.39230 sec, speed  581.54%
Default in 0.39818 sec, speed  572.95%
```
