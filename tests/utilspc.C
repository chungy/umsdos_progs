/*
	Help function for utstspc
*/
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include "umsdos_test.h"

/*
	Call system() after formatting the command line
	Return -1 if any error.
*/
static int utilspc_system (
	UMSDOS_REGISTER &reg,
	const char *ctl,
	...)
{
	va_list list;
	va_start (list,ctl);
	char buf[1000];
	vsprintf (buf,ctl,list);
	va_end (list);
	reg.verbose ("%s\n",buf);
	int ret = 0;
	if (system(buf)!=0){
		ret = -1;
		reg.prterr ("%s : failed, errno = %d (%s)\n",buf,errno
			,strerror(errno));
	}
	return ret;
}

/*
	Check if a program is locatable and executable
*/
static int utilspc_exeok(const char *name)
{
	char *path = getenv("PATH");
	int ret = 0;
	if (path == NULL){
		fprintf (stderr,"No variable PATH available\nCan't continue\n");
		ret = -1;
	}else{
		int ok = 0;
		while (1){
			while (isspace(*path))path++;
			char *start = path;
			while (*path != ':' && *path != '\0') path++;
			if (path == start){
				break;
			}else{
				int len = (int)(path-start);
				char dpath[PATH_MAX];
				memcpy (dpath,start,len);
				dpath[len] = '\0';
				strcat (dpath,"/");
				strcat (dpath,name);
				if (access(dpath,X_OK)==0){
					printf ("Found %s\n",dpath);
					ok = 1;
					break;
				}
				if (*path == ':') path++;
			}
		}
		if (!ok){
			fprintf (stderr
				,"Can't find program %s anywhere.\n"
				 "This program is needed for the tests.\n"
				 "Can't continue\n",name);
			ret = -1;
		}
	}
	return ret;
}

/*
	Check if all ressource are available before the test begin.
*/
void utilspc_check()
{
	/* #Specification: utstspc / what's needed
		utstspc use different other program to achieve its test.
		It has to reformat, mount, unmount etc... the floppy
		on which it is doing the test.

		utstspc assume that the following utility are available.

		/usr/bin/mformat
		/etc/mount or /bin/mount
		/etc/umount or /bin/umount

		Also, it requieres /etc/mtools
	*/
	int ret = utilspc_exeok("mformat");
	ret |= utilspc_exeok("mount");
	ret |= utilspc_exeok("umount");
	ret |= utilspc_exeok("umssync");
	if (access("/etc/mtools.conf",R_OK)!=0){
		fprintf (stderr,"Can't access /etc/mtools.conf, needed for the tests\n");
		ret = -1;
	}
	if (ret != 0) exit (-1);
}


class STARTUP{
public:
	STARTUP(){ utilspc_check(); }
};

STARTUP startup_utilspc;	// Force initialisation

/*
	Find the DOS drive letter associate with a device
*/
static int utilspc_getdevice (const char *drive, char *device)
{
	FILE *fin = fopen ("/etc/mtools.conf","r");
	int ret = -1;
	if (fin == NULL){
		fprintf (stderr,"Can't open /etc/mtools.conf: %s\n",strerror(errno));
		exit (-1);
	}else{
		char buf[300];
		while (fgets(buf,sizeof(buf)-1,fin)!=NULL){
			char *pt = buf;
			while (isspace(*pt)) if (*pt) break; else pt++;
			if ((pt[0] != '#') && (pt[0] != '\n')){	/* skip empty lines and comments */
				char dev[300];
				char drv[300];
				char t_drive[300];
				char *t_end;
				if (sscanf (pt,"%s %s %s",t_drive, drv, dev)!=3
					|| strlen(drv) != 2 || strcasecmp (t_drive, "drive")!=0){
					fprintf (stderr,"Unrecognized line (normal?) in /etc/mtools.conf\n\t%s\n"
						,buf);
				}else if ((toupper(drv[0]) == toupper(drive[0])) && (drv[1] == ':')) {
					if (strncasecmp(dev, "file=\"", 6) != 0) {
						fprintf (stderr, "Urecognized drive definition (hmmm?) in /etc/mtools.conf\n\t%s\n", buf); 
					} else {
						for (t_end=&dev[6]; *t_end != '"'; t_end++) {
							device[t_end-&dev[6]] = *t_end;
						}
						device[t_end-&dev[6]] = '\0';
//						fprintf (stderr, "/mn/ extracted device=%s\n", device);
						ret = 0;
						break;
					}
				}
			}
		}
		fclose (fin);
		if (ret == -1){
			fprintf (stderr,"Can't locate the DOS drive letter %s "
				"in /etc/mtools.conf\n"
				,drive);
			exit (-1);
		}
	}
	return ret;
}

/*
	Find out if a DOS DRIVE letter points to a floppy.
	Return != 0 if yes.
*/
int utilspc_isfloppy (const char *drive)
{
	char device[PATH_MAX];
	return utilspc_getdevice(drive,device)==0
		&& strncmp(device,"/dev/fd",7)==0;
}
/*
	Format a drive in DOS (FAT).

	Return -1 if any error.
*/
int utilspc_format (
	const char *drive,		// DOS drive letter.
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	if (utilspc_isfloppy(drive)){
		ret = utilspc_system (reg,"mformat %c:",drive[0]);
	}
	return ret;
}
/*
	Mount an msdos or umsdos partition

	Return -1 if error.
*/
int utilspc_mount (
	const char *drive,		// Device special
	const char *path,		// Mount point
	int umsdos,			// umsdos type or msdos
	const char *option,		// Mount options or NULL
	UMSDOS_REGISTER &reg)
{
	char device[PATH_MAX];
	int ret = -1;
	if (utilspc_getdevice(drive,device)!=-1){
		char optstr[100];
		optstr[0] = '\0';
		if (option != NULL) sprintf (optstr,"-o %s",option);
		ret = utilspc_system (reg,"mount -t %s %s %s %s"
			,umsdos ? "umsdos" : "msdos"
			,optstr,device,path);
	}
	return ret;
}

/*
	Unmount a file system.

	Return -1 if error.
*/
int utilspc_umount (
	const char *path,		// Mount point
	UMSDOS_REGISTER &reg)
{
	int ret = utilspc_system (reg,"umount %s",path);
	return ret;
}
/*
	Apply umssync in a directory

	Return -1 if any error.
*/
int utilspc_usync (const char *path, UMSDOS_REGISTER &reg)
{
	return utilspc_system (reg,"umssync %s",path);
}

/*
	Create a directory and make sure it is of type UMSDOS by
	applying umssync on it.

	Return -1 if any error.
*/
int utilspc_mkdir_udos(
	const char *dpath,
	int mode,
	UMSDOS_REGISTER &reg)
{
	int ret = util_mkdir (dpath,mode,reg,0);
	if (ret != -1) ret = utilspc_usync(dpath,reg);
	return ret;
}


