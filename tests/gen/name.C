/*
 *  umsdos_test/name.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS name mangling testing
*/
#include "umsdos_gen.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "ums_config.h"
#include "umsdos_test.h"
#include "umsdos_gen.h"


#ifndef KERN_22X
REGISTER (name_long,"Test file name too long");

static int name_long (const char *path)
{
	char fname[MAXSIZ_PATH+300];
	strcpy (fname,path);
	strcat (fname,"/");
	char *pt = fname + strlen(fname);
	int i;
	for (i=0; i<280; i++) pt[i] = 'a';
	pt[i] = '\0';
	Rname_long.verbose ("Creating a file with a name too long\n");
	util_create(fname,0777,Rname_long,0);
	struct stat stat;
	util_stat (fname,&stat,Rname_long,0);
	pt[UMSDOS_MAXNAME] = '\0';
	util_stat (fname,&stat,Rname_long,0);
	util_unlink (fname,Rname_long,0);
	return Rname_long.getnberr();
}
#endif

REGISTER (name_linux,"Create a --linux-.--- file");

static int name_linux (const char *)
{
	Rname_linux.verbose ("not done\n");
	return Rname_linux.getnberr();
}

REGISTER (name_dosdev,"Create file with DOS reserved device names");

static int name_dosdev (const char *)
{
	Rname_dosdev.verbose ("not done\n");
	return Rname_dosdev.getnberr();
}

