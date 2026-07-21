## Miscellaneous files to submit jobs to the DISI HPC cluster.
##
## Last modified 2026-02-25
## Moreno Marzolla <https://www.unibo.it/sitoweb/moreno.marzolla>

DISTFILES:=$(wildcard *.cu *.c *.sh *.md) Makefile
DISTNAME:=hpc-cluster
ALL: hpc-cluster-access.html

.PHONY: clean

%.html: %.md
	pandoc -s -V lang=en-US --from markdown --to html5 $< > $@

$(DISTNAME).zip: $(DISTFILES)
	\rm -r -f $(DISTNAME)
	mkdir $(DISTNAME)
	cd $(DISTNAME)
	ln $(DISTFILES) $(DISTNAME)
	zip -r $(DISTNAME).zip $(DISTNAME)

dist: $(DISTNAME).zip

clean:
	\rm -r -f *~ hpc-cluster-access.html

distclean: clean
	\rm -r -f $(DISTNAME) $(DISTNAME).zip

