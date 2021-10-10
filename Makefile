.PHONY: default all clean bench bench-u8 bench-u16 bench-u32 count_u8_bench count_u8_bench_cpp

default: all

SRCFILES = $(wildcard ./*.c)
OBJFILES = $(SRCFILES:.c=.o)
CFLAGS  ?= -O3 -std=c99 -fPIE -g

$(V)$(VERBOSE).SILENT:  # V=1 or VERBOSE=1 enables verbose mode.

all: bench-all

clean:
	$(RM) $(OBJFILES)
	$(RM) count_bench
	$(RM) count_bench_cpp
	$(RM) asm-listing.s

bench-all: count_bench count_bench_cpp
	echo count_bench     && ./count_bench
	echo count_bench_cpp && ./count_bench_cpp

bench: count_bench
	./count_bench

bench-u8: count_bench
	./count_bench --u8

bench-u16: count_bench
	./count_bench --u16

count_bench: $(OBJFILES)
	$(CC) -o $@ $^

count_bench_cpp: $(OBJFILES)
	$(CXX) -o $@ $^

asm-listing: $(OBJFILES)
	objdump -d -M intel -S $(OBJFILES) > asm-listing.s
