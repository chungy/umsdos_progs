#ifndef UMSDOS_PROGS_H
#define UMSDOS_PROGS_H

#include <sys/types.h>
#include <ums_config.h>


#define UPROG_VERSION	1
#define UPROG_RELEASE	32
#define PROG_ID(n)	fprintf(stderr,n " %d.%d "\
			"(Umsdos compatibility level %d.%d)\n"\
			,UPROG_VERSION,UPROG_RELEASE\
			,UMSDOS_VERSION,UMSDOS_RELEASE);

#include <sys/stat.h>
#include <dirent.h>
#include "umsdos_gen.h"
#include "umsdos_progs.p"

#if BE_UVFAT
#define DOSCTL_NAME "uvfatctl"
#define SETUP_NAME "uvfatsetup"
#define SYNC_NAME "uvfatsync"
#else
#define DOSCTL_NAME "udosctl"
#define SETUP_NAME "umssetup"
#define SYNC_NAME "umssync"
#endif

#endif

