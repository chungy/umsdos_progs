/*
 *  umsdos_test/file.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS hard link testing
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "umsdos_test.h"

/*
	Read a file and check it contains "hello\n".
*/
static void hlink_checkhello (
	const char *fname,
	UMSDOS_REGISTER &reg)
{
	if (reg.getnberr()==0){
		FILE *fin = util_fopen (fname,"r",reg);
		if (fin != NULL){
			char buf[100];
			if (fgets(buf,sizeof(buf)-1,fin)!=NULL){
				if (strcmp(buf,"hello\n")!=0){
					reg.prterr ("Can't read correctly \"hello\\n\","
						" got %s\n",buf);
				}
			}else{
				reg.prterr ("Can't read back \"hello\\n\"\n");
			}
			fclose (fin);	
		}
	}
}


/*
	Test many hlinkink chain to point to a file fpath
*/
static void hlink_many (
	const char *fpath,		// target of the link
	const char *lpath,		// base name for hard link
	int fpath_exist,		// fpath does exist or not
	UMSDOS_REGISTER &reg)
{
	if (reg.getnberr()==0){
		/* #Specification: utstgen / hard links / case / link 2 link 2 link ...
			hlink_simple does test a link made to a link made to a link
			and so on. On a normal UNIX file system, this test is not
			really an issue. Given the fact that a hardlink on UMSDOS is
			a symlink to a hidden file, it make sense to test at least
			the two cases:

			hard link to an existing file with no link
			hard link to an existing file with more than one link.
		*/
		int i;
		for (i=0; i<6 && reg.getnberr()==0; i++){
			char spath1[MAXSIZ_PATH];
			strcpy (spath1,lpath);
			// when i==0, spath1 == lpath
			int j;
			for (j=0; j<i; j++) strcat (spath1,".s");
			char spath2[MAXSIZ_PATH];
			strcpy (spath2,lpath);
			for (j=0; j<=i; j++) strcat (spath2,".s");
			if (fpath_exist){
				util_hardlink (spath1,spath2,reg,0);
				struct stat info;
				if (util_stat (spath2,&info,reg,0)!=-1
					&& info.st_size != 6){
					reg.prterr (
						"Invalid dimension for %s\n",spath2);
				}
				hlink_checkhello (spath2,reg);
			}else{
				util_hardlink (spath1,spath2,reg,ENOENT);
			}
		}
		if (fpath_exist){
			// Cleanup
			char spath[MAXSIZ_PATH];
			strcpy (spath,lpath);
			for (i=0; i<=5; i++){
				strcat (spath,".s");
				util_unlink (spath,reg,0);
			}
		}
	}
}


REGISTER (hlink_simple,"Simple hard link operations");

static int hlink_simple (const char *basepath)
{
	char dpath[MAXSIZ_PATH];
	sprintf (dpath,"%s/dir",basepath);
	if (util_mkdir (dpath,0777,Rhlink_simple,0)!=-1){
		char fpath[MAXSIZ_PATH];
		sprintf (fpath,"%s/file",dpath);
		FILE *fout = util_fopen (fpath,"w",Rhlink_simple);
		if (fout != NULL){
			fprintf (fout,"hello\n");
			fclose (fout);
			hlink_many (fpath,fpath,1,Rhlink_simple);
			/* #Specification: utstgen / hard link / cases / across directory boundary
				The target of the link is not in the same directory
				as the new link.
			*/
			char ddpath[MAXSIZ_PATH];
			sprintf (ddpath,"%s/sdir",dpath);
			if (util_mkdir (ddpath,0777,Rhlink_simple,0)!=-1){
				char spath[MAXSIZ_PATH];
				sprintf (spath,"%s/file",ddpath);
				hlink_many (fpath,spath,0,Rhlink_simple);
				util_rmdir (ddpath,Rhlink_simple,0);
			}
			util_unlink (fpath,Rhlink_simple,0);
			/* #Specification: utstgen / hard link / cases / target does not exist
				Many hard links are attempted to a file which
				does not exist.
			*/
			hlink_many (fpath,fpath,0,Rhlink_simple);
		}
		/* #Specification: utstgen / hard link / to a directory
			A hard link can't be made to a directory.
		*/
		char spath[MAXSIZ_PATH];
		sprintf (spath,"%s/error",basepath);
		util_hardlink (dpath,spath,Rhlink_simple,EPERM);
		util_rmdir (dpath,Rhlink_simple,0);
	}
	return Rhlink_simple.getnberr();
}







