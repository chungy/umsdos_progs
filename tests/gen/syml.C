/*
 *  umsdos_test/syml.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS Test symbolic links
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "umsdos_test.h"
#include "ums_config.h"

/*
	Read a file and check it contains "hello\n".
*/
static void syml_checkhello (
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


#ifdef KERN_22X
#define MAXSYML 4 /* this is hardcoded in for loop in do_follow_link in linux/fs/namei.c [MAXSYML+1] */
#else
#define MAXSYML 6
#endif

/*
	Test many symlink chain to point to a file fpath
*/
static void syml_many (
	const char *fpath,
	int fpath_exist,		// fpath does exist or not
	UMSDOS_REGISTER &reg)
{
	if (reg.getnberr()==0){
		/* #Specification: utstgen / symbolic links / link 2 link 2 link ...
			syml_simple does test the number of connected symlink the
			kernel can handle (A symlink pointing to another pointing
			to another ... and finally pointing to something.
		*/
		int i;
		for (i=0; i<MAXSYML && reg.getnberr()==0; i++){
			char spath1[MAXSIZ_PATH];
			strcpy (spath1,fpath);
			// when i==0, spath1 == fpath
			int j;
			for (j=0; j<i; j++) strcat (spath1,".s");
			char spath2[MAXSIZ_PATH];
			strcpy (spath2,fpath);
			for (j=0; j<=i; j++) strcat (spath2,".s");
			util_symlink (spath1,spath2,reg,0);
			if (i==5){
				struct stat info;
				util_stat (spath2,&info,reg,ELOOP);
			}else if (fpath_exist){
				struct stat info;
				if (util_stat (spath2,&info,reg,0)!=-1
					&& info.st_size != 6){
					reg.prterr (
						"Invalid dimension for %s\n",spath2);
				}
				syml_checkhello (spath2,reg);
			}else{
				struct stat info;
				util_stat (spath2,&info,reg,ENOENT);
			}
		}
		// Cleanup
		char spath[MAXSIZ_PATH];
		strcpy (spath,fpath);
		for (i=0; i<=MAXSYML-1; i++){
			strcat (spath,".s");
			util_unlink (spath,reg,0);
		}
	}
}


REGISTER (syml_simple,"Simple symbolic links operations");

static int syml_simple (const char *basepath)
{
	char dpath[MAXSIZ_PATH];
	sprintf (dpath,"%s/dir",basepath);
	if (util_mkdir (dpath,0777,Rsyml_simple,0)!=-1){
		char fpath[MAXSIZ_PATH];
		sprintf (fpath,"%s/file",dpath);
		FILE *fout = util_fopen (fpath,"w",Rsyml_simple);
		if (fout != NULL){
			fprintf (fout,"hello\n");
			fclose (fout);
			syml_many (fpath,1,Rsyml_simple);
			util_unlink (fpath,Rsyml_simple,0);
			syml_many (fpath,0,Rsyml_simple);
		}
		util_rmdir (dpath,Rsyml_simple,0);
	}
	return Rsyml_simple.getnberr();
}






