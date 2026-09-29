CFLAGS+=-O2 -std=c99 -Wall -Wpedantic

.PHONY: test clean distclean

ALL: serial-correlogram gen

gen: LDLIBS=-lm

test: ALL
	./gen 2 1000000 data1M
	./serial-correlogram data1M_noisy.txt out1M.txt 2048
	gnuplot plot.gp

clean:
	\rm -f serial-correlogram gen out1M.* data1M_*.txt

distclean: clean
