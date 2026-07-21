CFLAGS+=-O2 -std=c99 -Wall -Wpedantic

.PHONY: test clean distclean

ALL: correlogram gen

gen: LDLIBS=-lm

test: ALL
	./gen 2 1000000 data1M
	./correlogram data1M_noisy.txt out1M.txt 2048
	gnuplot plot.gp

clean:
	\rm -f correlogram gen out1M.* data1M_*.txt

distclean: clean
