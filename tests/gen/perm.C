/*
 *  umsdos_test/perm.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS Test permissions maintenance and ownership control.
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "umsdos_test.h"

/*
	Change selectivly the owner of a file, and then for each owner
	change group, and then for each pairs owner,group, flip through
	the permissions bits.
*/
static void perm_setowner (const char *path, UMSDOS_REGISTER &reg)
{
	util_chown (path,0,0,reg,0);
	for (int u=0; u<10; u++){
		util_chown (path,u,-1,reg,0);
		for (int g=0; g<10; g++){
			util_chown (path,-1,g,reg,0);
			static mode_t tb[]={
				0,07,077,0777,02777,0707,0666,0555
			};
			for (int m=0; m<sizeof(tb)/sizeof(tb[0]); m++){
				util_chmod (path,tb[m],reg,0);
			}
		}
		util_chown (path,-1,0,reg,0);
	}
}

REGISTER (perm_simple,"Change permissions and owner of file: must be root");

static int perm_simple (const char *basepath)
{
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/file",basepath);
	util_create (path,0777,Rperm_simple,0);
	perm_setowner (path,Rperm_simple);
	util_unlink (path,Rperm_simple,0);
	sprintf (path,"%s/dir",basepath);
	util_mkdir  (path,0777,Rperm_simple,0);
	perm_setowner (path,Rperm_simple);
	sprintf (path,"%s/dir/file",basepath);
	util_create (path,0777,Rperm_simple,0);
	perm_setowner (path,Rperm_simple);
	util_unlink (path,Rperm_simple,0);
	sprintf (path,"%s/dir",basepath);
	util_rmdir  (path,Rperm_simple,0);
	return Rperm_simple.getnberr();
}

REGISTER (perm_root,"Change permissions and owner of root directory: must be root");

static int perm_root (const char *basepath)
{
	perm_setowner (basepath,Rperm_root);
	return Rperm_root.getnberr();
}


