/*
 *  umsdos_test/pipe.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS named pipe testing
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "umsdos_test.h"

REGISTER (pipe_simple,"Basic test on named pipe");

static int pipe_simple (const char *path)
{
	const char *msg = "hello world";
	char fname[MAXSIZ_PATH];
	int pid;
	sprintf (fname,"%s/pipe",path);
	util_mkfifo(fname,0777,Rpipe_simple,0);
	util_mkfifo(fname,0777,Rpipe_simple,EEXIST);
	util_unlink(fname,Rpipe_simple,0);
	util_mkfifo(fname,0777,Rpipe_simple,0);
	pid = fork();
	if (pid==0){
		FILE *fout;
		sleep(1);
		fout = fopen (fname,"w");
		if (fout != NULL){
			fprintf (fout,msg);
			fclose (fout);
		}
		_exit(0);
	}else if (pid != -1){
		FILE *fin = util_fopen (fname,"r",Rpipe_simple);
		if (fin != NULL){
			char buf[100];
			if (fgets(buf,sizeof(buf)-1,fin)!=NULL){
				if (strcmp(buf,msg)!=0){
					Rpipe_simple.prterr ("Pipe message mismatch: %s != %s\n"
						,msg,buf);
				}
			}else{
				Rpipe_simple.prterr ("Empty message ???\n");
			}
			fclose (fin);
		}
		wait (NULL);
	}else{
		Rpipe_simple.prterr ("Can't fork\n");
	}
	util_unlink(fname,Rpipe_simple,0);
	util_unlink(fname,Rpipe_simple,ENOENT);
	return Rpipe_simple.getnberr();
}

	



