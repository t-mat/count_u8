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

```
$ gcc count_u8_bench.c && ./a.out
Scalar  in 2.20600 sec, speed  100.00%
SSE2    in 0.14200 sec, speed 1553.52%
Default in 0.13500 sec, speed 1634.07%
```
