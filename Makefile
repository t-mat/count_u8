.PHONY: default all clean bench

SRCFILES  = $(wildcard ./*.c)
OBJFILES  = $(SRCFILES:.c=.o)
CFLAGS   ?= -O3

default: all

# Silent mode by default.  Verbose mode can be triggered by V=1 or VERBOSE=1.
$(V)$(VERBOSE).SILENT:

all: bench

clean:
	$(RM) $(OBJFILES)
	$(RM) count_u8_bench

bench: count_u8_bench
	./count_u8_bench

count_u8_bench: $(OBJFILES)
	$(CC) count_u8_bench.c -o $@
