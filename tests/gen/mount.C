/*
 *  umsdos_test/mount.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS mounting file system in UMSDOS
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "umsdos_test.h"

REGISTER (mount_simple,"Mount a file system in a subdirectory");

static int mount_simple(const char *basepath)
{
	Rmount_simple.verbose ("Not done yet, test empty and hide\n");
	return Rmount_simple.getnberr();
}

