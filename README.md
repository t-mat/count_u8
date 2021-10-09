# count_u8
Count the number of uint8_t elements in memory region.  Header-only library in C99.  Optimized for SSE2.

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

`count_u8()` automatically detect supported instruction by compiler's
predefined symbols such as `__SSE2__`, `_M_X64`.


## Benchmark

### gcc

```
$ gcc-11 -O3 count_u8_bench.c && ./a.out
Scalar  in 1.75201 sec, speed  100.00%
SSE2    in 0.18753 sec, speed  934.26%
Default in 0.19031 sec, speed  920.61%
```

```
$ gcc-9 -O3 count_u8_bench.c && ./a.out
Scalar  in 1.85213 sec, speed  100.00%
SSE2    in 0.14778 sec, speed 1253.31%
Default in 0.15283 sec, speed 1211.86%
```


### clang

```
$ clang-12 -O3 count_u8_bench.c && ./a.out
Scalar  in 3.50516 sec, speed  100.00%
SSE2    in 0.15010 sec, speed 2335.19%
Default in 0.15099 sec, speed 2321.46%
```

```
$ clang-10 -O3 count_u8_bench.c && ./a.out
Scalar  in 2.87657 sec, speed  100.00%
SSE2    in 0.16849 sec, speed 1707.28%
Default in 0.16848 sec, speed 1707.33%
```


## MSVC

```
# MSVC 2019
Scalar  in 2.19493 sec, speed  100.00%
SSE2    in 0.15442 sec, speed 1421.39%
Default in 0.15047 sec, speed 1458.67%
```
