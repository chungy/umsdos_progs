/*
 *  umsdos_test/dir.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS directory testing
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "umsdos_test.h"
#include "ums_config.h"

REGISTER (dir_simple,"Create simple directory");

static int dir_simple(const char *basepath)
{
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/dir",basepath);
	Rdir_simple.verbose ("Creating directory %s\n",path);
	if (util_mkdir(path,0777,Rdir_simple,0)!=-1){
		if (util_dir_count (path,Rdir_simple)!=2){
			Rdir_simple.prterr ("Empty directory not empty ??\n");
		}
		util_mkdir  (path,0777,Rdir_simple,EEXIST);
		util_create (path,0666,Rdir_simple,EISDIR);
		char fpath[MAXSIZ_PATH];
		sprintf (fpath,"%s/file1",path);
		util_create (fpath,0666,Rdir_simple,0);
		util_mkdir  (fpath,0777,Rdir_simple,EEXIST);
		util_rmdir (path,Rdir_simple,ENOTEMPTY);
		util_rmdir (fpath,Rdir_simple,ENOTDIR);
		util_unlink (fpath,Rdir_simple,0);
		util_mkdir (path,0777,Rdir_simple,EEXIST);
		util_unlink (path,Rdir_simple,EISDIR);
		util_rmdir (path,Rdir_simple,0);
	}
	return Rdir_simple.getnberr();
}

static void dir_removeall (
	const char *basepath,
	const char *recur,	// Sub-directory name
	int count,
	UMSDOS_REGISTER &reg)
{
	/* Remove all subdirectory */
	for (int i=count-1; i>0 && reg.getnberr()==0; i--){
		char path[MAXSIZ_PATH];
		strcpy (path,basepath);
		for (int j=0; j<i; j++){
			strcat (path,"/");
			strcat (path,recur);
		}
		util_rmdir (path,reg,0);
	}
}

static void dir_recur(
	const char *basepath,
	const char *recur,
	UMSDOS_REGISTER &reg)
{
	for (int i=0; i<10 && reg.getnberr()==0; i++){
		char path[MAXSIZ_PATH];
		strcpy (path,basepath);
		for (int j=0; j<i; j++){
			strcat (path,"/");
			strcat (path,recur);
		}
		if (i == 0 || util_mkdir(path,0777,reg,0)!=-1){
			if (dir_simple (path) == -1){
				reg.prterr ("Failure at level %d\n",i);
				break;
			}
		}
	}
	dir_removeall (basepath,recur,10,reg);
}

REGISTER (dir_recur,"Apply simple test on sub-directory recursivly");

static int dir_recur(const char *basepath)
{
	// Goes 10 level deep
	Rdir_recur.verbose ("Doing a 10 levels test\n");
	dir_recur (basepath,"recur",Rdir_recur);
	dir_recur (basepath,"RECURSIVE",Rdir_recur);
	return Rdir_recur.getnberr();
}

REGISTER (dir_walk,"Create a tree and walk in it");

static int dir_walk(const char *basepath)
{
	#define RECUR_NAME "RECUR"
	Rdir_walk.verbose ("Creating directories %s/"
		RECUR_NAME "/" RECUR_NAME "/" RECUR_NAME "\n"
		,basepath);
	for (int i=1; i<4 && Rdir_walk.getnberr()==0; i++){
		char path[MAXSIZ_PATH];
		strcpy (path,basepath);
		for (int j=0; j<i; j++) strcat (path,"/" RECUR_NAME);
		util_mkdir(path,0777,Rdir_walk,0);
	}
	if (Rdir_walk.getnberr()==0){
		char wpath[MAXSIZ_PATH];
		struct stat info1;
		sprintf (wpath,"%s/" RECUR_NAME
			"/./../"
			RECUR_NAME "/" RECUR_NAME "/" RECUR_NAME
			"/../../../" RECUR_NAME
			,basepath);
		util_stat (wpath,&info1,Rdir_walk,0);
	}
	dir_removeall (basepath,RECUR_NAME,4,Rdir_walk);
	return Rdir_walk.getnberr();
}

REGISTER (dir_spcent,"Create special entries (. and ..) in directory");

static void dir_spcent_test(
	const char *path,
	UMSDOS_REGISTER &reg)
{
	char spath[MAXSIZ_PATH];
	sprintf (spath,"%s/.",path);
	char sspath[MAXSIZ_PATH];
	sprintf (sspath,"%s/..",path);
	util_mkdir(spath,0777,reg,EEXIST);
	util_mkdir(sspath,0777,reg,EEXIST);
#ifndef KERN_22X
	util_rmdir(spath,reg,EPERM);
	util_rmdir(sspath,reg,EPERM);
#endif
}

static int dir_spcent(const char *basepath)
{
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/dir",basepath);
	Rdir_spcent.verbose ("Creating directory %s\n",path);
	/* #Specification: utstgen / creating . and ..
		A check is done that the special entries . and .. can't be
		created nor removed.
	*/
	if (util_mkdir(path,0777,Rdir_spcent,0)!=-1){
		dir_spcent_test (path,Rdir_spcent);
		dir_spcent_test (basepath,Rdir_spcent);
		util_rmdir (path,Rdir_spcent,0);
	}
	return Rdir_spcent.getnberr();
}

#ifndef KERN_22X
REGISTER (dir_removebusy,"Try to remove a busy directory");

static int dir_removebusy(const char *basepath)
{
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/dir",basepath);
	Rdir_spcent.verbose ("Creating directory %s\n",path);
	/* #Specification: utstgen / removing a busy directory
		A check is done that a busy directory can't be removed.
		Here is the sequence we test. It must fail with EBUSY.
		
		mkdir dir
		cd dir
		rm ../dir
	*/
	if (util_mkdir(path,0777,Rdir_removebusy,0)!=-1){
		char curdir[MAXSIZ_PATH];
		getcwd(curdir,sizeof(curdir)-1);
		if (util_chdir(path,Rdir_removebusy,0)!=-1){
			util_rmdir (path,Rdir_removebusy,EBUSY);
			util_chdir (curdir,Rdir_removebusy,0);
		}
		util_rmdir (path,Rdir_removebusy,0);
	}
	return Rdir_removebusy.getnberr();
}
#endif
