/*
 *  umsdos_test/multi.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS multi task test.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "umsdos_test.h"



/*
	Creates one file "fileNO" and return != 0 for success
*/
static int multi_createone (
	const char *basepath,
	int no)
{
	char path[MAXSIZ_PATH];
	sprintf (path,"%s/file%d",basepath,no);
	int ret = open (path,O_RDWR|O_CREAT|O_EXCL,0666);
	if (ret != -1){
		close (ret);
		ret = 1;
	}else{
		if (errno != EEXIST){
			perror ("err");
		}
		ret = 0;
	}
	return ret;
}
/*
	Creates nb file (file1 -> fileNB) either in ascending order
	or descending.
*/
static void multi_createmany(
	const char *basepath,
	int nb,
	int ascending,
	int task,		// who am i
	int result[])	// Will hold the status (0 or 1) of the creation
			// for each of the nb file
{
	if (ascending){
		for (int i=0; i<nb; i++){
			result[i] = multi_createone (basepath,i);
			printf ("%d",task); fflush(stdout);
		}
	}else{
		for (int i=nb-1; i >= 0; i--){
			result[i] = multi_createone (basepath,i);
			printf ("%d",task); fflush(stdout);
		}
	}
}
static int multi_testpath(const char *basepath, UMSDOS_REGISTER &reg)
{
	/* #Specification: utstgen / multi task / basic test
		A simple test is performed on a directory, by many task.
		Only one task must succeeded at a time. The others must fail
		with specific error code.

		So we fork 10 time.

		This test hopes it is a sufficient test :-(
	*/
	const int NB_TASK=10;
	const int NB_CREATE=100;	// Number of file to create
	int pid[NB_TASK];
	int fdres[NB_TASK];
	reg.verbose ("Multi task test in directory %s\n",basepath);
	// Check that all unlink were successful (just in case)
	int i;
	for (i=0; i<NB_CREATE; i++){
		char path[MAXSIZ_PATH];
		struct stat info;
		sprintf (path,"%s/file%d",basepath,i);
		if (stat(path,&info)!=-1){
			reg.prterr ("file %s already exist ???\n",path);
		}
	}
	if (reg.getnberr()==0){
		for (i=0; i<NB_TASK; i++){
			fdres[i] = -1;
			pid[i] = -1;
			int fd[2];
			if (pipe(fd) == -1){
				reg.prterr ("Can't create pipe for task %d\n",i);
				break;
			}else{
				int pd = pid[i] = fork();
				if (pd == -1){
					reg.prterr ("Can't fork task %d\n",i);
					break;
				}else if (pd == 0){
					close (fd[0]);
					int result[NB_CREATE];
					multi_createmany (basepath,NB_CREATE,i&1,i,result);
					write (fd[1],result,sizeof(result));
					_exit(0);
				}else{
					fdres[i] = fd[0];
					close (fd[1]);
				}
			}
		}
		if (i < NB_TASK){
			for (int t=0; t<i; t++){
				close (fdres[t]);
				kill (pid[t],SIGKILL);
			}
		}else{
			// Collect the result back
			int res[NB_TASK][NB_CREATE];
			memset (res,-1,sizeof(res));
			for (i=0; i<NB_TASK; i++){
				int nbread = read (fdres[i],res[i],sizeof(res[i]));
				if (nbread != sizeof(res[i])){
					reg.prterr ("Invalid read from task: %d != %d\n"
						,nbread,sizeof(res[i]));
					break;
				}
			}
			for (int t=0; t<NB_TASK; t++) close (fdres[t]);
			if (i==NB_TASK){
				// Only one task may have succeed
				for (i=0; i<NB_CREATE; i++){
					int count = 0;
					for (int t=0; t<NB_TASK; t++){
						if (res[t][i]) count++;
					}
					char path[MAXSIZ_PATH];
					sprintf (path,"%s/file%d",basepath,i);
					struct stat info;
					if (count != 1){
						reg.prterr ("%d tasks succeeded : file %d\n"
							,count,i);
					}else if (stat(path,&info)==-1){
						reg.prterr ("File %s is missing\n",path);
					}
				}
				if (reg.getnberr()==0){
					for (i=0; i<NB_CREATE; i++){
						char path[MAXSIZ_PATH];
						sprintf (path,"%s/file%d",basepath,i);
						if (unlink(path)==-1){
							reg.prterr ("Can't unlink file %s\n",path);
						}
					}
					// Check that all unlink were successful (just in case)
					for (i=0; i<NB_CREATE; i++){
						char path[MAXSIZ_PATH];
						struct stat info;
						sprintf (path,"%s/file%d",basepath,i);
						if (stat(path,&info)!=-1){
							reg.prterr ("file %s still exist ???\n",path);
						}
					}
				}
			}
		}
	}
	return reg.getnberr();
}


REGISTER (multi_simple,"Simple multi task testing");


static int multi_simple(const char *basepath)
{
	int ret = multi_testpath (basepath, Rmulti_simple);
	if (ret == 0){
		// Do the same test in a subdirectory
		char path[MAXSIZ_PATH];
		sprintf (path,"%s/dir",basepath);
		if (util_mkdir(path,0777,Rmulti_simple,0)!=-1){
			multi_testpath (path,Rmulti_simple);
			util_rmdir (path,Rmulti_simple,0);
		}
	}
	return Rmulti_simple.getnberr();
}

