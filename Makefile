.PHONY: default all clean bench count_u8_bench count_u8_bench_cpp

default: all

SRCFILES = $(wildcard ./*.c)
OBJFILES = $(SRCFILES:.c=.o)
CFLAGS  ?= -O3 -std=c99 -fPIE

$(V)$(VERBOSE).SILENT:  # V=1 or VERBOSE=1 enables verbose mode.

all: bench

clean:
	$(RM) $(OBJFILES)
	$(RM) count_bench
	$(RM) count_bench_cpp

bench: count_bench count_bench_cpp
	echo count_bench     && ./count_bench
	echo count_bench_cpp && ./count_bench_cpp

count_bench: $(OBJFILES)
	$(CC) -o $@ $^

count_bench_cpp: $(OBJFILES)
	$(CXX) -o $@ $^
