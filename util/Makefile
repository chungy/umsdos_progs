GCC=gcc
INC=-I../include
CFLAGS=$(INC) -pipe -O2 -g -Wall -Wno-implicit-function-declaration -Wno-implicit #-w

all: .depend testver_min testver udump umssync udosctl umssetup
other: 

install: all
	install -m 755 -o bin -g bin -s umssync /sbin
	ln -sf /sbin/umssync /sbin/udosctl
	ln -sf /sbin/umssync /sbin/umssetup
	install -m 755 -o bin -g bin umssync.8 /usr/man/man8

# Installation in the L.A.D.R directories
SBIN_LADR=/n/pc_jac/diska/umsdos/base/admin/sbin
install_kit: all
	install --strip -m 755 -o bin -g bin umssync $(SBIN_LADR)
	( cd $(SBIN_LADR); \
		ln -sf umssync udosctl; \
		ln -sf umssync umssetup; \
	)

# Linux Utilities
OBJS = main.o umssync.o umssetup.o udosctl.o umsdosio.o mangle.o\
		numconst.o printk.o
umssync: $(OBJS)
	$(GCC) -o umssync $(OBJS)

udosctl: umssync
	ln -sf umssync udosctl

umssetup: umssync
	ln -sf umssync umssetup

# Not really useful utilities
# Dump of a --linux-.---
udump: udump.c mangle.o printk.o
	$(GCC) $(CFLAGS) -o udump udump.c mangle.o printk.o

# test UMSDOS_GETVERSION (Basic communication with kernel). Totally useless.
testver: testver.o umsdosio.o
	$(GCC) $(CFLAGS) -o testver testver.o umsdosio.o

testver_min: testver_min.c ../include/umsdos_fs.h
	$(GCC) $(CFLAGS) -o testver_min testver_min.c

.c.o:
	$(GCC) $(CFLAGS) -c $< -o $@

# Using directly one module from the kernel!
mangle.o: /usr/src/linux/fs/umsdos/mangle.c
	@echo $<
	@$(GCC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.old *~ *.bak umssync udump udosctl umssetup testver testver_min mkhead .depend tags

tags:
	ctags --format=1 *.c 

dep:
	-$(GCC) $(INC) -MM *.c > .depend

.depend:
	-$(GCC) $(INC) -MM *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
