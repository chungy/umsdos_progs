GPP=g++
GCC=gcc
MAKECMD=$(MAKE) GCC=$(GCC) GPP=$(GPP)
all:
	$(MAKECMD) -C include all
	$(MAKECMD) -C util all
	$(MAKECMD) -C tests utstgen utstspc

install:
	$(MAKECMD) -C include all
	$(MAKECMD) -C util install

clean:
	$(MAKECMD) -C include clean
	$(MAKECMD) -C util clean
	$(MAKECMD) -C tests clean
	rm -rf *~

dist: clean include/getver.sh
	cd .. && tar zcvf umsdos_progs-`umsdos_progs/include/getver.sh`.tgz umsdos_progs

