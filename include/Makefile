GCC=gcc
CFLAGS=-pipe -O2 -g #-Wall #-w

all: umsdos_gen.h
	true


mkhead: mkhead.c umsdos_fs.h
	$(GCC) $(CFLAGS) -o mkhead mkhead.c

#umsdos_gen.h: mkhead
#	rm -f umsdos_gen.h
#	./mkhead > umsdos_gen.h

clean:
	rm -f mkhead *~
