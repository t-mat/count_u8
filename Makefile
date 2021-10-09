.PHONY: default all clean bench count_u8_bench count_u8_bench_cpp

default: all

SRCFILES = $(wildcard ./*.c)
OBJFILES = $(SRCFILES:.c=.o)
CFLAGS  ?= -O3 -std=c99

$(V)$(VERBOSE).SILENT:  # V=1 or VERBOSE=1 enables verbose mode.

all: bench

clean:
	$(RM) $(OBJFILES)
	$(RM) count_u8_bench
	$(RM) count_u8_bench_cpp

bench: count_u8_bench count_u8_bench_cpp
	./count_u8_bench
	./count_u8_bench_cpp

count_u8_bench: $(OBJFILES)
	$(CC) count_u8_bench.c -o $@

count_u8_bench_cpp: $(OBJFILES)
	$(CXX) count_u8_bench.c -o $@
