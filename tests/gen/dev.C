/*
 *  umsdos_test/dev.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS device special testing
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "umsdos_test.h"

REGISTER (dev_tty,"Create a tty device and use it");

static int dev_tty(const char *basepath)
{
	/* #Specification: utstgen / testing tty device
		To test somewhat the ability of umsdos to handle
		special device file, we create a tty device (/dev/tty)
		with the following command:
		
		mknod tty c 5 0
		
		This represent the general tty (pointing to your own
		terminal). We create it few time, and ask the user
		to enter "hello" and check if we are getting this
		string.
	*/
	const int major = 5;
	const int minor = 0;
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/tty",basepath);
	util_mkdev (path,0777,1,major,minor,Rdev_tty,0);
	util_unlink (path,Rdev_tty,0);
	util_mkdev (path,0777,1,major,minor,Rdev_tty,0);
	util_mkdev (path,0777,1,major,minor,Rdev_tty,EEXIST);
	util_create (path,0777,Rdev_tty,EEXIST);
	FILE *fin = util_fopen (path,"r",Rdev_tty);
	if (fin != NULL){
		printf ("Could you type hello please ? "); fflush (stdout);
		char buf[30];
		if (fgets (buf,sizeof(buf)-1,fin)!=NULL){
			if (strcmp("hello\n",buf)!=0){
				Rdev_tty.prterr ("tty read invalid %s != %s","hello\n",buf);
			}
		}
		fclose (fin);
	}
	util_unlink (path,Rdev_tty,0);
	return Rdev_tty.getnberr();
}

