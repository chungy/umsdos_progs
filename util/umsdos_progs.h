#ifndef UMSDOS_PROGS_H
#define UMSDOS_PROGS_H

#include <sys/types.h>

#define UPROG_VERSION	1
#define UPROG_RELEASE	11
#define PROG_ID(n)	fprintf(stderr,n " %d.%d "\
			"(Umsdos compatibility level %d.%d)\n"\
			,UPROG_VERSION,UPROG_RELEASE\
			,UMSDOS_VERSION,UMSDOS_RELEASE);

#include <sys/stat.h>
#include <dirent.h>
#include "umsdos_gen.h"
#include "umsdos_progs.p"

#endif

