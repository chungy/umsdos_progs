/*
 *  umsdos_test/time.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS Test time stamp control (access, modification and creation)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "umsdos_test.h"

static char *time_ctime (time_t tim, char *buf)
{
	strcpy (buf,ctime(&tim));
	return buf;
}
/*
	Compare for approximate equality between 2 time with error reporting.
	Compare for close equality because of different delays which
	prevents total accuracy.
*/
static void time_checkea (
	time_t ftime,	// One time stamp of a file
	time_t cmp,		// Time to compare
	UMSDOS_REGISTER &reg,
	const char *msg)
{
	time_t diff = labs(ftime - cmp);
	if (diff > 2){
		char ftime_str[50];
		char cmp_str[50];
		reg.prterr ("Unexpected %s time :\n"
			"\t     got %s"
			"\texpected %s",msg
			,time_ctime(ftime,ftime_str)
			,time_ctime(cmp,cmp_str));
	}
}
/*
	Compare for exact equality between 2 time with error reporting.
*/
static void time_checke (
	time_t ftime,	// One time stamp of a file
	time_t cmp,		// Time to compare
	UMSDOS_REGISTER &reg,
	const char *msg)
{
	if (ftime != cmp){
		char ftime_str[50];
		char cmp_str[50];
		reg.prterr ("Unexpected changed %s time :\n"
			"\t     got %s"
			"\texpected %s",msg
			,time_ctime(ftime,ftime_str)
			,time_ctime(cmp,cmp_str));
	}
}

/*
	Validate if any of the time stamp of a file is "now"
*/
static void time_checknow (
	const char *path,
	struct stat &old_info,	// Contain the old time stamp
							// and are check against the new info
							// if one check_xxx flag is off, it is expected
							// that the corresponding time has not changed
							// since the previous stat operation
	UMSDOS_REGISTER &reg,
	int check_ctime,	// Check creation time ?
	int check_mtime,	// Check modification time ?
	int check_atime)	// Check access time ?
{
	if (reg.getnberr()==0){
		time_t timenow;
		time (&timenow);
		struct stat info;
		util_stat (path,&info,reg,0);
		// Check the 3 times
		if (check_ctime){
			time_checkea (info.st_ctime,timenow,reg,"ctime");
		}else{
			time_checke (info.st_ctime,old_info.st_ctime,reg,"ctime");
		}
		if (check_mtime){
			time_checkea (info.st_mtime,timenow,reg,"mtime");
		}else{
			time_checke (info.st_mtime,old_info.st_mtime,reg,"mtime");
		}
		if (check_atime){
			time_checkea (info.st_atime,timenow,reg,"atime");
		}else{
			time_checke (info.st_atime,old_info.st_atime,reg,"atime");
		}
		old_info = info;
	}
}

/*
	Wait 4 seconds if there was no error.
*/
static void time_sleep (UMSDOS_REGISTER &reg)
{
	if (reg.getnberr()==0) sleep(4);
}


REGISTER (time_simple,"Simple operation to change the time stamp of files");

static int time_simple (const char *basepath)
{
	/*
		We create a file into a new sub-directory and apply
		different operation on it. We then check for the file and the
		directory the 3 time stamp.

		This routine uses very long sleep time (see time_sleep())
		because I have done most of the test on floppy.
	*/
	Rtime_simple.verbose ("ATTENTION: This test must be done on a quiet\n");
	Rtime_simple.verbose ("           machine!\n");
	char dpath[MAXSIZ_PATH];
	sprintf (dpath,"%s/dir",basepath);
	if (util_mkdir (dpath,0777,Rtime_simple,0)!=-1){
		// Make sure all 2 times are set
		struct stat dinfo;
		util_stat (dpath,&dinfo,Rtime_simple,0);
		time_checknow (dpath,dinfo,Rtime_simple,1,1,1);
		// Create the file
		time_sleep (Rtime_simple);
		char fpath[MAXSIZ_PATH];
		sprintf (fpath,"%s/file",dpath);
		util_create (fpath,0777,Rtime_simple,0);
		struct stat finfo;
		util_stat (fpath,&finfo,Rtime_simple,0);
		time_checknow (fpath,finfo,Rtime_simple,1,1,1);
		time_checknow (dpath,dinfo,Rtime_simple,1,1,0);
		time_sleep (Rtime_simple);
		int fout = util_open (fpath,O_WRONLY|O_TRUNC,Rtime_simple,0);
		if (fout != -1){
			// The file was already empty, so no change is expected
			time_checknow (fpath,finfo,Rtime_simple,1,1,0);
			time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
			time_sleep (Rtime_simple);
			Rtime_simple.verbose ("write\n");
			write (fout,"hello\n",7);
			time_checknow (fpath,finfo,Rtime_simple,1,1,0);
			time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
			time_sleep (Rtime_simple);
			Rtime_simple.verbose ("close\n");
			close (fout);
			time_checknow (fpath,finfo,Rtime_simple,0,0,0);
			time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
			time_sleep (Rtime_simple);
			int fin = util_open (fpath,O_RDONLY,Rtime_simple,0);
			if (fin != -1){
				time_checknow (fpath,finfo,Rtime_simple,0,0,0);
				time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
				time_sleep (Rtime_simple);
				char buf[100];
				Rtime_simple.verbose ("read\n");
				int nb = read (fin,buf,sizeof(buf));
				if (nb != 7){
					Rtime_simple.prterr ("Can't read back %s\n",fpath);
				}else{
					time_checknow (fpath,finfo,Rtime_simple,0,0,1);
					time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
				}
				Rtime_simple.verbose ("close\n");
				close (fin);
			}
		}		
		time_sleep (Rtime_simple);
		util_chown (fpath,1,1,Rtime_simple,0);
		time_checknow (fpath,finfo,Rtime_simple,1,0,0);
		time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
		time_sleep (Rtime_simple);
		util_chmod (fpath,0777,Rtime_simple,0);
		time_checknow (fpath,finfo,Rtime_simple,1,0,0);
		time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
		time_sleep (Rtime_simple);
		util_unlink (fpath,Rtime_simple,0);
		time_checknow (dpath,dinfo,Rtime_simple,1,1,0);
		time_sleep (Rtime_simple);
		util_unlink (fpath,Rtime_simple,ENOENT);
		time_checknow (dpath,dinfo,Rtime_simple,0,0,0);
		// Sub-directory creation
		time_sleep (Rtime_simple);
		char spath[MAXSIZ_PATH];
		sprintf (spath,"%s/sdir",dpath);
		util_mkdir (spath,0777,Rtime_simple,0);
		time_checknow (dpath,dinfo,Rtime_simple,1,1,0);
		time_sleep (Rtime_simple);
		util_rmdir (spath,Rtime_simple,0);
		time_checknow (dpath,dinfo,Rtime_simple,1,1,0);
		util_rmdir (dpath,Rtime_simple,0);
	}
	return Rtime_simple.getnberr();
}





