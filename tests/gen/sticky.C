/*
 *  umsdos_test/gen/sticky.c
 *
 *  Written 1995 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS sticky bit on directory: Test permissions
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "umsdos_test.h"
#include "ums_config.h"

struct STICKY_PATHS{
	char dir1[MAXSIZ_PATH];
	char dir2[MAXSIZ_PATH];
	char file1[MAXSIZ_PATH];
	char file2[MAXSIZ_PATH];
};

struct STICKY_USER {
	int uid;
	int gid;
};
struct STICKY_MODOWN {
	int uid;
	int gid;
	int mode;
};


/*
	Setup the various paths used by the different tests
*/
static void sticky_setpaths (STICKY_PATHS &p, const char *basepath)
{
	sprintf (p.dir1,"%s/dir1",basepath);
	sprintf (p.dir2,"%s/dir2",basepath);
	sprintf (p.file1,"%s/file1",p.dir1);
	sprintf (p.file2,"%s/file2",p.dir2);
}

/*
	Create one directory with specific ID and permissions.
*/
static int sticky_mkdir (
	const char *dir,
	STICKY_MODOWN &s,
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	if (util_mkdir (dir,0777,reg,0) != -1
		&& util_chown (dir,s.uid,s.gid,reg,0) != -1
		&& util_chmod (dir,s.mode,reg,0)!=-1){
		ret = 0;
	}
	return ret;
}
/*
	Create two directories in basepath
*/
static int sticky_mkdirs (
	STICKY_PATHS &p,
	STICKY_MODOWN &s1,
	STICKY_MODOWN &s2,
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	if (sticky_mkdir (p.dir1,s1,reg)!=-1
		&& sticky_mkdir (p.dir2,s2,reg) != -1){
		ret = 0;
	}
	return ret;
}
/*
	Remove two directories in basepath
*/
static int sticky_rmdirs (
	STICKY_PATHS &p,
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	unlink (p.file1);
	unlink (p.file2);
	if (util_rmdir (p.dir1,reg,0)!=-1
		&& util_rmdir (p.dir2,reg,0) != -1){
		ret = 0;
	}
	return ret;
}
/*
	Create one directory with specific ID and permissions.
*/
static int sticky_create (
	const char *file,
	STICKY_MODOWN &s,
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	if (s.uid == -1){	// Tricks to mean no creation needed
						// See #define NO	-1 below
		ret = 0;
	}else if (util_create (file,0777,reg,0) != -1
		&& util_chown (file,s.uid,s.gid,reg,0) != -1
		&& util_chmod (file,s.mode,reg,0)!=-1){
		ret = 0;
	}
	return ret;
}
static int sticky_create(
	STICKY_PATHS &p,
	STICKY_MODOWN &s1,
	STICKY_MODOWN &s2,
	UMSDOS_REGISTER &reg)
{
	int ret = -1;
	if (sticky_create(p.file1,s1,reg)!=-1
		&& sticky_create(p.file2,s2,reg)!=-1){
		ret = 0;
	}
	return ret;
}

/*
	Check if the current effective user id is allowed to delete a file
	with given permissions/ownership in a directory with given permissions
	ownership.
*/
static int util_delok (
	struct stat &infofile,
	struct stat &infodir)
{
	int ret = EPERM;
	int euid = geteuid();
	int egid = getegid();

//	printf ("[check: UID %d/%d, GID %d/%d]\n", euid, infodir.st_uid, egid, infodir.st_gid);

	if (euid == 0) ret=0;			/* root can do everything */
	
	if (infodir.st_mode & S_ISVTX) {	/* if directory is STICKY */
		if (euid == (int) infodir.st_uid)		/* we are owner of dir */
			if (infodir.st_mode & S_IWUSR) ret=0; 	/* and it is writeable by USER */

#ifndef KERN_22X	/* for 2.2.10, if sticky, only UID matters, not GID. */
		if (egid == (int) infodir.st_gid)		/* we are group of dir */
			if (infodir.st_mode & S_IWGRP) ret=0; 	/* and it is writeable by GROUP */
#endif			
		if (infodir.st_mode & S_IWOTH)	 			/* if it is writeable by OTHER, */
			if (euid == (int) infofile.st_uid) ret=0;	/* AND we are owner of file */
			
	} else {				/* directory is NOT sticky */
		if (euid == (int) infodir.st_uid)		/* we are owner of dir */
			if (infodir.st_mode & S_IWUSR) ret=0; 	/* and it is writeable by USER */
		if (egid == (int) infodir.st_gid)		/* we are group of dir */
			if (infodir.st_mode & S_IWGRP) ret=0; 	/* and it is writeable by GROUP */
		if (infodir.st_mode & S_IWOTH) ret=0; 		/* or it is writeable by OTHER */
	}

	return ret;
}

/*
	Do a rename, but try to compute the expected result.
	Instead of doing a big test case table, I prefer trying almost
	all combination and do a simple algorythm to compute the expect result
	from the kernel.
*/
static int sticky_renchk (
	STICKY_PATHS &p,
	UMSDOS_REGISTER &reg)
{
	struct stat infodir1;
	struct stat infofile1;
	struct stat infodir2;
	struct stat infofile2;
	int ret = -1;
	if (util_stat(p.dir1,&infodir1,reg,0)!=-1
		&& util_stat(p.file1,&infofile1,reg,0)!=-1
		&& util_stat(p.dir2,&infodir2,reg,0)!=-1){
		/* The destination file may exist or not */
		int err_expected = 0;
		if (util_delok(infofile1,infodir1)==EPERM) err_expected = EPERM;
		if (stat(p.file2,&infofile2)!=-1){
			if (util_delok(infofile2,infodir2)==EPERM) err_expected = EPERM;
		}
//		printf ("/expect=%d/ ", err_expected);
		ret = util_rename (p.file1,p.file2,reg,err_expected);
	}
	return ret;
}
static int sticky_ren (
	STICKY_PATHS &p,
	STICKY_USER &u,		// Switch to this uid before doing the test
	UMSDOS_REGISTER &reg)
{
//printf ("u %d %d ",u.uid,u.gid);
	setegid (u.gid);
	seteuid (u.uid);
	int ret = sticky_renchk (p,reg);
	seteuid (0);
	setegid (0);
	return ret;
}

static int sticky_sequence (
	STICKY_PATHS &p,
	STICKY_MODOWN &dir1,
	STICKY_MODOWN &file1,
	STICKY_MODOWN &dir2,
	STICKY_MODOWN &file2,
	STICKY_USER &renamer,
	UMSDOS_REGISTER &reg)
{
	if (sticky_mkdirs(p,dir1,dir2,reg) != -1
		&& sticky_create(p,file1,file2,reg) != -1){
		util_chmod (p.dir1,dir1.mode,reg,0);
		util_chmod (p.dir2,dir2.mode,reg,0);
		sticky_ren (p,renamer,reg);
		if (reg.getnberr()) exit (99);
		
		sticky_rmdirs (p,reg);
	}
	return reg.getnberr()==0 ? 0 : -1;
}

REGISTER (sticky_rename,"Rename a file in a directory with sticky bit on");

static int sticky_rename (const char *basepath)
{
	printf ("The sticky bit test takes forewer (8192 cases)\n"
			"Do you want to run it ? [n] ");
	fflush (stdout);
	char str[10];
	if (fgets(str,sizeof(str)-1,stdin)==NULL
		|| str[0] == 'n' || str[0] == 'N' || str[0] == '\n') return 0;
	STICKY_PATHS p;
	sticky_setpaths (p,basepath);
	/* #Specification: utstgen / rename / sticky bit
		Quite a few tests are done on the rename function call
		related to sticky bits. We create directory own by
		different users, putting or not sticky bits on, with different
		permissions and we try to rename (move) one file to the other.
		The destination may exist or not.
	*/
	for (int i=0; i<2; i++){
		int sticky_dir1 = i==1 ? 0 : S_ISVTX;
		for (int j=0; j<2; j++){
			int sticky_dir2 = j==1 ? 0 : S_ISVTX;
			for (int dst_exist=0; dst_exist<2; dst_exist++){
				// All variation of uid/gid paires
				static STICKY_USER tbu[]={
						{0,0},
						{0,MYGID},
						{MYUID,0},
						{MYUID,MYGID},
				};
				for (int src_down=0; src_down <4; src_down++){
					for (int src_fown=0; src_fown <4; src_fown++){
						for (int dst_down=0; dst_down <4; dst_down++){
							for (int dst_fown=0; dst_fown <4; dst_fown++){
								for (int user=0; user <4; user++){
									STICKY_MODOWN dir1;
									dir1.uid = tbu[src_down].uid;
									dir1.gid = tbu[src_down].gid;
									dir1.mode = 0777 | sticky_dir1;
									STICKY_MODOWN file1;
									file1.uid = tbu[src_fown].uid;
									file1.gid = tbu[src_fown].gid;
									file1.mode = 0777;
									STICKY_MODOWN dir2;
									dir2.uid = tbu[dst_down].uid;
									dir2.gid = tbu[dst_down].gid;
									dir2.mode = 0777 | sticky_dir2;
									STICKY_MODOWN file2;
									file2.uid = tbu[dst_fown].uid;
									file2.gid = tbu[dst_fown].gid;
									file2.mode = 0777;
									if (!dst_exist) file2.uid = -1;
									sticky_sequence (p
										,dir1,file1
										,dir2,file2
										,tbu[user]
										,Rsticky_rename);
								}
							}
						}
					}
				}
			}
		}
	}
	return Rsticky_rename.getnberr();
}



