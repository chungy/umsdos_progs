#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include "umsdos_progs.h"
#include "umsdos_gen.h"


/* #Specification: umsdos_progs / udosctl
	The udosctl utility give acces directly to UMSDOS ioctl on directory.

	udosctl command arg

	Here are the commands:

	ls:

		List the content of dos directory arg. Bypass the EMD file.
		It uses UMSDOS_READDIR_DOS.

	create:

		Create the file arg in the EMD file. Do nothing on the DOS
		directory. Use UMSDOS_CREAT_UMSDOS.

	mkdir:

		Create the directory arg in the EMD file. Do nothing on the DOS
		directory. Use UMSDOS_CREAT_UMSDOS.

	rm:

		Remove the file arg in the DOS directory. Bypass the EMD file.
		Use UMSDOS_UNLINK_DOS.

	rmdir:

		Remove the directory arg in the DOS directory. Bypass the EMD file.
		Use UMSDOS_RMDIR_DOS.

	uls:

		List the content of the EMD and print the corresponding DOS
		mangled name. It uses UMSDOS_READDIR_EMD.
		
	urm:

		Remove the file arg from the EMD file. Don't touch the DOS
		directory. Use UMSDOS_UNLINK_UMSDOS.

	urmdir:

		Remove the directory arg from the EMD file. Don't touch the DOS
		directory. Use UMSDOS_UNLINK_UMSDOS.

	version:

		Prints the version of the UMSDOS driver running.

	This program was done mostly for illustration of ioctl use
	and testing.
*/

/*
	Split a path in a directory component and file name component.

	If the path has no directory component, put ".".
*/
static void parse_path (
	const char *path,
	char *dpath,
	char *fname)
{
	char *pt = dpath;
	char *lastpt=NULL;
	strcpy (dpath,path);
	while ((pt=strchr(pt,'/'))!=NULL){
		lastpt = pt;
		pt++;
	}
	if (lastpt == NULL){
		strcpy (fname,dpath);
		strcpy (dpath,".");
	}else{
		*lastpt++ = '\0';
		strcpy (fname,lastpt);
	}
	printf ("parse :%s: -> :%s: :%s:\n",path,dpath,fname);
}

int udosctl_main (int argc, char *argv[])
{
	int ret = -1;
	if (argc != 3){
		PROG_ID("udosctl");
		fprintf (stderr,
			"udosctl command argument\n"
			"\n"
			"udosctl ls directory_path\n"
			"\n"
			"udosctl create file_name\n"
			"udosctl mkdir directory_name\n"
			"\n"
			"udosctl rm file_path\n"
			"udosctl rmdir directory_path\n"
			"\n"
			"udosctl uls directory_path\n"
			"\n"
			"udosctl urm file_path\n"
			"udosctl urmdir directory_path\n"
			"\n"
			"udosctl stat file_path\n"
			"\n"
			"udosctl version directory_path"
			"\n"
			"udosctl is part of the umsdos file system\n"
			);
	}else{
		char *cmd = argv[1];
		char *arg = argv[2];
		int fd;
		
		if (strcmp(cmd,"ls")==0){
			struct dirent dirent;
			fd = UM_open (arg,1,1);
			while (UM_readdir (fd, &dirent) != -1) printf ("%s\n", dirent.d_name);
		}else if (strcmp(cmd,"uls")==0){
			struct dirent dirent;
			struct umsdos_dirent udirent;
			fd = UM_open (arg,1,1);
			while (UM_ureaddir(fd, &udirent,&dirent)!=-1){
				printf ("%s -> %s\n",udirent.name,dirent.d_name);
			}
		}else if(strcmp(cmd,"version")==0){
			int version, release;
			fd = UM_open (arg,1,1);
			UM_getversion (fd, &version, &release);
			printf ("Umsdos version %d.%d\n",version,release);
		}else{
			char dpath[PATH_MAX];
			char fname[PATH_MAX];
			parse_path (arg,dpath,fname);
			fd = UM_open (dpath,1,1);
			if (strcmp(cmd,"create")==0){
				ret = UM_create (fd, fname,S_IFREG|0666,0,0,0,0,0,0);
			}else if (strcmp(cmd,"mkdir")==0){
				ret = UM_create (fd, fname,S_IFDIR|0666,0,0,0,0,0,0);
			}else if (strcmp(cmd,"rm")==0){
				ret = UM_dosunlink (fd, fname);
			}else if (strcmp(cmd,"rmdir")==0){
				ret = UM_dosrmdir (fd, fname);
			}else if (strcmp(cmd,"urmdir")==0){
				ret = UM_urmdir (fd, fname);
			}else if (strcmp(cmd,"urm")==0){
				ret = UM_uunlink (fd, fname);
			}else if (strcmp(cmd,"stat")==0){
				struct stat fstat;
				ret = UM_dosstat (fd, fname, &fstat);
				if (ret != -1){
					printf ("\tino   = %ld\n",fstat.st_ino);
					printf ("\tmode  = %o\n",fstat.st_mode);
					printf ("\tatime = %ld\n",fstat.st_atime);
					printf ("\tctime = %ld\n",fstat.st_ctime);
					printf ("\tmtime = %ld\n",fstat.st_mtime);
				}
			}else{
				ret = -1;
				fprintf (stderr,"Invalid command %s\n",cmd);
			}
		}
	}
	return ret;
}

