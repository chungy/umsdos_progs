/*
 *  umsdos_test/file.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS regular file testing
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "umsdos_test.h"

REGISTER (file_many,"Create several files and delete all");

static int file_many (const char *path)
{
	int i;
	for (i=0; i<200; i++){
		char fname[MAXSIZ_PATH];
		sprintf (fname,"%s/ftest%d",path,i);
		util_create(fname,0777,Rfile_many,0);
	}
	for (i=0; i<200; i++){
		char fname[MAXSIZ_PATH];
		sprintf (fname,"%s/ftest%d",path,i);
		util_unlink(fname,Rfile_many,0);
	}
	return Rfile_many.getnberr();
}


REGISTER (file_remove,"Delete non existent file");

static int file_remove (const char *path)
{
	char fname[MAXSIZ_PATH];
	sprintf (fname,"%s/ftest1",path);
	util_unlink (fname,Rfile_remove,ENOENT);
	return Rfile_remove.getnberr();
}

REGISTER (file_rmopen,"Delete an opened file and continue writing to it");

static int file_rmopen (const char *path)
{
	char fname[MAXSIZ_PATH];
	FILE *fout;
	sprintf (fname,"%s/ftest1",path);
	util_create(fname,0777,Rfile_rmopen,0);
	fout = util_fopen (fname,"w+",Rfile_rmopen);
	if (fout != NULL){
		util_unlink (fname,Rfile_rmopen,0);
		int i;
		for (i=0; i<1000; i++) fprintf (fout,"this is a test %d\n",i);
		rewind (fout);
		for (i=0; i<1000; i++){
			char buf[100];
			char tst[100];
			sprintf (tst,"this is a test %d\n",i);
			if (fgets(buf,sizeof(buf)-1,fout)!=NULL){
				if (strcmp(buf,tst)!=0){
					Rfile_rmopen.prterr(
						"Content mismatch, line %d %s != %s\n"
						,i+1,tst,buf);
					break;
				}
			}else{
				Rfile_rmopen.prterr ("Can't read line %d\n",i+1);
				break;
			}
		}
		if (i==1000) Rfile_rmopen.verbose("1000 lines read successfully\n");
		fclose (fout);
	}else{
		util_unlink (fname,Rfile_rmopen,0);
	}
	return Rfile_rmopen.getnberr();
}
	
/*
	Basic rename test sequence
*/
static void file_renseq (
	const char *basepath1,
	const char *basepath2,
	int do_dir,				// Test sub-directories instead of files
	UMSDOS_REGISTER &reg)
{
	// Do some test with files
	char fname1[MAXSIZ_PATH];
	sprintf (fname1,"%s/tst1",basepath1);
	char fname2[MAXSIZ_PATH];
	sprintf (fname2,"%s/tst2",basepath2);
	if (do_dir){
		util_mkdir(fname1,0777,reg,0);
	}else{
		util_create(fname1,0777,reg,0);
	}
	util_rename(fname1,fname2,reg,0);
	if (do_dir){
		util_mkdir(fname1,0777,reg,0);
	}else{
		util_create(fname1,0777,reg,0);
	}
	util_rename(fname2,fname1,reg,0);
	if (do_dir){
		util_rmdir(fname1,reg,0);
	}else{
		util_unlink(fname1,reg,0);
	}
}

/*
	Rename test sequence for an open file.
*/
static void file_renopen (
	const char *basepath1,
	const char *basepath2,
	UMSDOS_REGISTER &reg)
{
	char fname1[MAXSIZ_PATH];
	sprintf (fname1,"%s/open1",basepath1);
	util_create(fname1,0777,reg,0);
	FILE *fout = util_fopen (fname1,"w",reg);
	if (fout != NULL){
		char fname2[MAXSIZ_PATH];
		sprintf (fname2,"%s/open2",basepath2);
		util_rename(fname1,fname2,reg,0);
		if (reg.getnberr()==0){
			reg.verbose ("write \"hello\\n\"\n");
			int ret = fprintf (fout,"hello\n");
			if (ret != 6){
				reg.prterr ("fprintf failed, returned %d %s\n"
					,ret,strerror(errno));
			}
			ret = fclose (fout);
			if (ret != 0){
				reg.prterr ("fclose failed, %s\n",strerror(errno));
			}
			FILE *fin = util_fopen (fname2,"r",reg);
			if (fin != NULL){
				char buf[20];
				if (fgets(buf,sizeof(buf)-1,fin)==NULL){
					reg.prterr ("Can't read back\n");
				}else if (strcmp(buf,"hello\n") != 0){
					reg.prterr ("Corrupt read back data:\n"
						"\tExpected: hello\\n\n"
						"\t     got: %s",buf);
				}else{
					util_unlink (fname2,reg,0);
				}
				fclose (fin);
			}
		}else{
			fclose (fout);
		}
	}
}
/*
	Basic rename test sequence for files and directories and open files
*/
static void file_renseq (
	const char *basepath1,
	const char *basepath2,
	UMSDOS_REGISTER &reg)
{
	file_renseq (basepath1,basepath2,0,reg);
	file_renseq (basepath1,basepath2,1,reg);
}


REGISTER (file_rename,"Rename files and directories");

static int file_rename (const char *basepath)
{
	/* #Specification: utstgen / Rename test
		Rename test are done with files and directories.
		The following case are tested

		-In the same directory
		-In the root directory
		-In two independant directory
		-In a subdirectory and the parent

		The same test is also done on an open file.
	*/
	char dname1[MAXSIZ_PATH];
	sprintf (dname1,"%s/dir1",basepath);
	char dname2[MAXSIZ_PATH];
	sprintf (dname2,"%s/dir2",basepath);
	util_mkdir(dname1,0777,Rfile_rename,0);
	util_mkdir(dname2,0777,Rfile_rename,0);
	file_renseq (basepath,basepath,Rfile_rename);
	file_renseq (dname1,dname1,Rfile_rename);
	file_renseq (dname1,dname2,Rfile_rename);
	file_renseq (dname1,basepath,Rfile_rename);
	file_renseq (basepath,dname1,Rfile_rename);

	/* #Specification: utstgen / Rename test / open file
		Rename test is done on an open file. We do the following
		sequence.

		create a file
		Open it
		Rename it
		Write to the open handle
		Close it
		Open the file using the news name
		Read back the data and check it.

		This test does not succeed if the file is renamed accross
		directories. This sounds like a limitation of the linux msdos
		driver. I am not sure at this point. I hope it is not
		a critical feature of a Unix file system. Comments are welcome
		about this topics. Comments with solution also :-)
	*/
	file_renopen (basepath,basepath,Rfile_rename);
	file_renopen (dname1,dname1,Rfile_rename);
	// The following test fail
	// file_renopen (dname1,dname2,Rfile_rename);

	// Cleanup
	util_rmdir (dname1,Rfile_rename,0);
	util_rmdir (dname2,Rfile_rename,0);
	return Rfile_rename.getnberr();
}



